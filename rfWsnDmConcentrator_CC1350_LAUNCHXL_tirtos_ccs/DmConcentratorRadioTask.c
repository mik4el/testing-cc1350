/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***** Includes *****/
#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <DmConcentratorRadioTask.h>
#include <seb/SEB.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>

/* Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>
#include <stdio.h>

/* Board Header files */
#include "Board.h"

#include "easylink/EasyLink.h"
#include "RadioProtocol.h"


/***** Defines *****/
#define CONCENTRATORRADIO_TASK_STACK_SIZE 1024
#define CONCENTRATORRADIO_TASK_PRIORITY   3

#define RADIO_EVENT_ALL                  0xFFFFFFFF
#define RADIO_EVENT_VALID_PACKET_RECEIVED      (uint32_t)(1 << 0)
#define RADIO_EVENT_INVALID_PACKET_RECEIVED (uint32_t)(1 << 1)

#define CONCENTRATORRADIO_MAX_RETRIES 2
#define NORERADIO_ACK_TIMEOUT_TIME_MS (160)
#define CONCENTRATOR_MAX_NODES 7

#define CONCENTRATOR_SUB1_ACTIVITY_LED Board_PIN_LED0
#define CONCENTRATOR_BLE_ACTIVITY_LED Board_PIN_LED1

#define CONCENTRATOR_0M_TXPOWER    -10

#define FRACT_BITS 8
#define INT2FIXED(x) (((uint16_t)x) << FRACT_BITS)

/***** Variable declarations *****/
static Task_Params concentratorRadioTaskParams;
Task_Struct concentratorRadioTask; /* not static so you can see in ROV */
static uint8_t concentratorRadioTaskStack[CONCENTRATORRADIO_TASK_STACK_SIZE];
Event_Struct radioOperationEvent;  /* not static so you can see in ROV */
static Event_Handle radioOperationEventHandle;

static ConcentratorRadio_PacketReceivedCallback packetReceivedCallback;
static union ConcentratorPacket latestRxPacket;
static EasyLink_TxPacket txPacket;
static struct AckPacket ackPacket;
static uint8_t concentratorAddress;
static int8_t latestRssi;
static struct SensorNodeRX latestActiveSensorNodeRX;
struct SensorNodeRX knownSensorNodeRXs[CONCENTRATOR_MAX_NODES];
static struct SensorNodeRX* lastAddedSensorNodeRX = knownSensorNodeRXs;

static ConcentratorAdvertiser bleAdvertiser = {
        CONCENTRATOR_ADVERTISE_INVALID,
        Concentrator_AdvertiserNone
};

struct SensorNodeRX {
    uint8_t address;
    uint32_t timeForLastRX;
};

static uint8_t bleMacAddr[6];

/***** Prototypes *****/
static void concentratorRadioTaskFunction(UArg arg0, UArg arg1);
static void rxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status);
static void notifyPacketReceived(union ConcentratorPacket* latestRxPacket);
static void sendAck(uint8_t latestSourceAddress);
static void sendBleAdvertisement(struct DualModeInternalTempSensorPacket sensorPacket);
static void sendEmptyBleAdvertisement(void);
static void addNewNodeRX(struct SensorNodeRX* node);
static void updateNodeRX(struct SensorNodeRX* node);
static uint8_t isKnownNodeAddress(uint8_t address);
static uint32_t timeForLastRXForAdress(uint8_t address);
static uint8_t numberOfNodes(void);

/* Pin driver handle */
static PIN_Handle ledPinHandle;
static PIN_State ledPinState;

/* Configure LED Pin */
PIN_Config ledPinTable[] = {
        CONCENTRATOR_SUB1_ACTIVITY_LED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
        CONCENTRATOR_BLE_ACTIVITY_LED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
#ifdef __CC1350_LAUNCHXL_BOARD_H__
        Board_DIO1_RFSW | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
        Board_DIO30_SWPWR | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
#endif //__CC1350_LAUNCHXL_BOARD_H__
    PIN_TERMINATE
};

/***** Function definitions *****/
void ConcentratorRadioTask_init(void) {

    /* Open LED pins */
    ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    if (!ledPinHandle)
    {
        System_abort("Error initializing board 3.3V domain pins\n");
    }

    /* Create event used internally for state changes */
    Event_Params eventParam;
    Event_Params_init(&eventParam);
    Event_construct(&radioOperationEvent, &eventParam);
    radioOperationEventHandle = Event_handle(&radioOperationEvent);

    /* Create the concentrator radio protocol task */
    Task_Params_init(&concentratorRadioTaskParams);
    concentratorRadioTaskParams.stackSize = CONCENTRATORRADIO_TASK_STACK_SIZE;
    concentratorRadioTaskParams.priority = CONCENTRATORRADIO_TASK_PRIORITY;
    concentratorRadioTaskParams.stack = &concentratorRadioTaskStack;
    Task_construct(&concentratorRadioTask, concentratorRadioTaskFunction, &concentratorRadioTaskParams, NULL);
}

void ConcentratorRadioTask_registerPacketReceivedCallback(ConcentratorRadio_PacketReceivedCallback callback) {
    packetReceivedCallback = callback;
}

void ConcentratorRadioTask_setAdvertiser(ConcentratorAdvertiser advertiser) {
    bleAdvertiser.sourceAddress = advertiser.sourceAddress;
    bleAdvertiser.type = advertiser.type;
}

static void concentratorRadioTaskFunction(UArg arg0, UArg arg1)
{
    /* Set multiclient mode for Prop Sub1G */
    EasyLink_setCtrl(EasyLink_Ctrl_MultiClient_Mode, 1);

    /* Initialize EasyLink */
    if (EasyLink_init(RADIO_EASYLINK_MODULATION) != EasyLink_Status_Success)
    {
        System_abort("EasyLink_init failed");
    }

    /* If you wish to use a frequency other than the default use
     * the below API
     * EasyLink_setFrequency(868000000);
     */

    /* Set concentrator address */
    concentratorAddress = RADIO_CONCENTRATOR_ADDRESS;
    EasyLink_enableRxAddrFilter(&concentratorAddress, 1, 1);

    /* Set up ack packet */
    ackPacket.header.sourceAddress = concentratorAddress;
    ackPacket.header.packetType = RADIO_PACKET_TYPE_ACK_PACKET;

    /* Initialise the Simple Beacon module called directly for Prop Adv
     * Set multiclient mode to true
     */
    SimpleBeacon_init(true);

    /* Initialise the Simple Eddystone Beacon module
     * Set multiclient mode to true
     */
    SEB_init(true);

    /* Set the Eddystone URL */
    SEB_initUrl("https://m4bd.se/", CONCENTRATOR_0M_TXPOWER);

    SimpleBeacon_getIeeeAddr(bleMacAddr);

#ifdef __CC1350_LAUNCHXL_BOARD_H__
    /* Enable power to RF switch to 2.4G antenna */
    PIN_setOutputValue(ledPinHandle, Board_DIO30_SWPWR, 1);
#endif //__CC1350_LAUNCHXL_BOARD_H__

    /* Enter receive */
    if (EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success) {
        System_abort("EasyLink_receiveAsync failed");
    }

    EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, EasyLink_ms_To_RadioTime(1000));

    while (1)
    {
        uint32_t events = Event_pend(radioOperationEventHandle, 0, RADIO_EVENT_ALL, BIOS_WAIT_FOREVER);

        /* If valid packet received */
        if (events & RADIO_EVENT_VALID_PACKET_RECEIVED)
        {

            /* toggle Sub1G Activity LED */
            PIN_setOutputValue(ledPinHandle, CONCENTRATOR_SUB1_ACTIVITY_LED,
                               !PIN_getOutputValue(CONCENTRATOR_SUB1_ACTIVITY_LED));

            /* Send ack packet */
            sendAck(latestRxPacket.header.sourceAddress);

            /* Call packet received callback */
            notifyPacketReceived(&latestRxPacket);

            /* If we knew this node from before, update the value */
            if (isKnownNodeAddress(latestActiveSensorNodeRX.address))
            {
                updateNodeRX(&latestActiveSensorNodeRX);
            }
            else
            {
                /* Else add it */
                addNewNodeRX(&latestActiveSensorNodeRX);
            }

            /* Go back to RX */
            if (EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success)
            {
                System_abort("EasyLink_receiveAsync failed");
            }

            /* toggle Sub1G Activity LED */
            PIN_setOutputValue(ledPinHandle, CONCENTRATOR_SUB1_ACTIVITY_LED,
                               !PIN_getOutputValue(CONCENTRATOR_SUB1_ACTIVITY_LED));

        }

        if ( (bleAdvertiser.type != Concentrator_AdvertiserNone) &&
             (latestRxPacket.header.packetType == RADIO_PACKET_TYPE_DM_SENSOR_PACKET) )
        {
            //send ble advertisement
            sendBleAdvertisement(latestRxPacket.dmSensorPacket);
        }

        if (bleAdvertiser.type == Concentrator_AdvertiserNone) {
            sendEmptyBleAdvertisement();
        }

        /* If invalid packet received */
        if (events & RADIO_EVENT_INVALID_PACKET_RECEIVED)
        {
            /* Go back to RX */
            if (EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success)
            {
                System_abort("EasyLink_receiveAsync failed");
            }
        }

    }
}

static uint8_t isKnownNodeAddress(uint8_t address) {
    uint8_t found = 0;
    uint8_t i;
    for (i = 0; i < CONCENTRATOR_MAX_NODES; i++)
    {
        if (knownSensorNodeRXs[i].address == address)
        {
            found = 1;
            break;
        }
    }
    return found;
}

static uint8_t numberOfNodes(void) {
    uint8_t i;
    for (i = 0; i < CONCENTRATOR_MAX_NODES; i++)
    {
        if (knownSensorNodeRXs[i].address == 0)
        {
            break;
        }
    }
    return i;
}

static uint32_t timeForLastRXForAdress(uint8_t address) {
    uint8_t i;
    for (i = 0; i < CONCENTRATOR_MAX_NODES; i++)
    {
        if (knownSensorNodeRXs[i].address == address)
        {
            return knownSensorNodeRXs[i].timeForLastRX;
        }
    }
    return 0;
}

static void updateNodeRX(struct SensorNodeRX* node) {
    uint8_t i;
    for (i = 0; i < CONCENTRATOR_MAX_NODES; i++) {
        if (knownSensorNodeRXs[i].address == node->address)
        {
            knownSensorNodeRXs[i].timeForLastRX = (Clock_getTicks() * Clock_tickPeriod) / 1000000;
            break;
        }
    }
}

static void addNewNodeRX(struct SensorNodeRX* node) {
    *lastAddedSensorNodeRX = *node;

    /* Increment and wrap */
    lastAddedSensorNodeRX++;
    if (lastAddedSensorNodeRX > &knownSensorNodeRXs[CONCENTRATOR_MAX_NODES-1])
    {
        lastAddedSensorNodeRX = knownSensorNodeRXs;
    }
}


static void sendBleAdvertisement(struct DualModeInternalTempSensorPacket sensorPacket)
{

#ifdef __CC1350_LAUNCHXL_BOARD_H__
    //Swtich RF switch to 2.4G antenna
    PIN_setOutputValue(ledPinHandle, Board_DIO1_RFSW, 0);
#endif //__CC1350_LAUNCHXL_BOARD_H__

    /* Toggle activity LED */
    PIN_setOutputValue(ledPinHandle, CONCENTRATOR_BLE_ACTIVITY_LED,!PIN_getOutputValue(CONCENTRATOR_BLE_ACTIVITY_LED));

    // Prepare TLM frame
    uint8_t txCnt, chan;
    char url_format[] = "https://m4bd.se/s/%02x/";
    char url_ready[21];
    sprintf(url_ready, url_format, sensorPacket.header.sourceAddress);
    SEB_initUrl(url_ready , CONCENTRATOR_0M_TXPOWER);
    uint32_t timeSinceLastRx = 0;
    if (timeForLastRXForAdress(sensorPacket.header.sourceAddress)!=0) {
        timeSinceLastRx = ((Clock_getTicks() * Clock_tickPeriod) / 1000000) - timeForLastRXForAdress(sensorPacket.header.sourceAddress);
    }

    SEB_initTLM(sensorPacket.batt, INT2FIXED(sensorPacket.internalTemp), timeSinceLastRx);

    for (txCnt = 0; txCnt < SimpleBeacon_AdvertisementTimes; txCnt++)
    {
        for (chan = 37; chan < 40; chan++)
        {
            SEB_sendFrame(SEB_FrameType_Url, bleMacAddr, 1, (uint64_t) 1<<chan);
            SEB_sendFrame(SEB_FrameType_Tlm, bleMacAddr, 1, (uint64_t) 1<<chan);
        }

        // Sleep on all but last advertisement
        if(txCnt+1 < SimpleBeacon_AdvertisementTimes)
        {
            Task_sleep(SimpleBeacon_AdvertisementIntervals[txCnt]);
        }
    }

#ifdef __CC1350_LAUNCHXL_BOARD_H__
    //Switch RF switch to Sub1G antenna
    PIN_setOutputValue(ledPinHandle, Board_DIO1_RFSW, 1);
#endif //__CC1350_LAUNCHXL_BOARD_H__

    /* Toggle activity LED */
    PIN_setOutputValue(ledPinHandle, CONCENTRATOR_BLE_ACTIVITY_LED,!PIN_getOutputValue(CONCENTRATOR_BLE_ACTIVITY_LED));
}

static void sendEmptyBleAdvertisement(void)
{

#ifdef __CC1350_LAUNCHXL_BOARD_H__
    //Swtich RF switch to 2.4G antenna
    PIN_setOutputValue(ledPinHandle, Board_DIO1_RFSW, 0);
#endif //__CC1350_LAUNCHXL_BOARD_H__

    /* Toggle activity LED */
    PIN_setOutputValue(ledPinHandle, CONCENTRATOR_BLE_ACTIVITY_LED,!PIN_getOutputValue(CONCENTRATOR_BLE_ACTIVITY_LED));

    // Prepare TLM frame
    uint8_t txCnt, chan;
    char url_format[] = "https://m4bd.se/c/%02x/";
    char url_ready[21];
    sprintf(url_ready, url_format, concentratorAddress);
    SEB_initUrl(url_ready , CONCENTRATOR_0M_TXPOWER);

    SEB_initTLM(0, INT2FIXED((uint32_t)numberOfNodes()), 0);

    for (txCnt = 0; txCnt < SimpleBeacon_AdvertisementTimes; txCnt++)
    {
        for (chan = 37; chan < 40; chan++)
        {
            SEB_sendFrame(SEB_FrameType_Url, bleMacAddr, 1, (uint64_t) 1<<chan);
            SEB_sendFrame(SEB_FrameType_Tlm, bleMacAddr, 1, (uint64_t) 1<<chan);
        }

        // Sleep on all but last advertisement
        if(txCnt+1 < SimpleBeacon_AdvertisementTimes)
        {
            Task_sleep(SimpleBeacon_AdvertisementIntervals[txCnt]);
        }
    }

#ifdef __CC1350_LAUNCHXL_BOARD_H__
    //Switch RF switch to Sub1G antenna
    PIN_setOutputValue(ledPinHandle, Board_DIO1_RFSW, 1);
#endif //__CC1350_LAUNCHXL_BOARD_H__

    /* Toggle activity LED */
    PIN_setOutputValue(ledPinHandle, CONCENTRATOR_BLE_ACTIVITY_LED,!PIN_getOutputValue(CONCENTRATOR_BLE_ACTIVITY_LED));
}

static void sendAck(uint8_t latestSourceAddress) {

    /* Set destinationAdress, but use EasyLink layers destination address capability */
    txPacket.dstAddr[0] = latestSourceAddress;

    /* Copy ACK packet to payload, skipping the destination address byte.
     * Note that the EasyLink API will implicitly both add the length byte and the destination address byte. */
    memcpy(txPacket.payload, &ackPacket.header, sizeof(ackPacket));
    txPacket.len = sizeof(ackPacket);

    /* Send packet */
    if (EasyLink_transmit(&txPacket) != EasyLink_Status_Success)
    {
        System_abort("EasyLink_transmit failed");
    }
}

static void notifyPacketReceived(union ConcentratorPacket* latestRxPacket)
{
    if (packetReceivedCallback)
    {
        packetReceivedCallback(latestRxPacket, latestRssi);
    }
    if (latestRxPacket->header.packetType == RADIO_PACKET_TYPE_DM_SENSOR_PACKET) {
        latestActiveSensorNodeRX.address = latestRxPacket->header.sourceAddress;
    }
}

static void rxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status)
{
    union ConcentratorPacket* tmpRxPacket;

    /* If we received a packet successfully */
    if (status == EasyLink_Status_Success)
    {
        /* Save the latest RSSI, which is later sent to the receive callback */
        latestRssi = (int8_t)rxPacket->rssi;

        /* Check that this is a valid packet */
        tmpRxPacket = (union ConcentratorPacket*)(rxPacket->payload);

        /* If this is a known packet */
        if (tmpRxPacket->header.packetType == RADIO_PACKET_TYPE_ADC_SENSOR_PACKET)
        {
            /* Save packet */
            latestRxPacket.header.sourceAddress = rxPacket->payload[0];
            latestRxPacket.header.packetType = rxPacket->payload[1];
            latestRxPacket.adcSensorPacket.adcValue = (rxPacket->payload[2] << 8) | rxPacket->payload[3];

            /* Signal packet received */
            Event_post(radioOperationEventHandle, RADIO_EVENT_VALID_PACKET_RECEIVED);
        }
        else if (tmpRxPacket->header.packetType == RADIO_PACKET_TYPE_DM_SENSOR_PACKET)
        {
            /* Save packet */
            latestRxPacket.header.sourceAddress = rxPacket->payload[0];
            latestRxPacket.header.packetType = rxPacket->payload[1];
            latestRxPacket.dmSensorPacket.adcValue = (rxPacket->payload[2] << 8) | rxPacket->payload[3];
            latestRxPacket.dmSensorPacket.batt = (rxPacket->payload[4] << 8) | rxPacket->payload[5];
            latestRxPacket.dmSensorPacket.internalTemp = (rxPacket->payload[6] << 24) |
                                                         (rxPacket->payload[7] << 16) |
                                                         (rxPacket->payload[8] << 8) |
                                                          rxPacket->payload[9];
            latestRxPacket.dmSensorPacket.time100MiliSec = (rxPacket->payload[10] << 24) |
                                                           (rxPacket->payload[11] << 16) |
                                                           (rxPacket->payload[12] << 8) |
                                                            rxPacket->payload[13];
            latestRxPacket.dmSensorPacket.button = rxPacket->payload[14];

            /* Signal packet received */
            Event_post(radioOperationEventHandle, RADIO_EVENT_VALID_PACKET_RECEIVED);
        }
        else
        {
            /* Signal invalid packet received */
            Event_post(radioOperationEventHandle, RADIO_EVENT_INVALID_PACKET_RECEIVED);
        }
    }
    else
    {
        /* Signal invalid packet received */
        Event_post(radioOperationEventHandle, RADIO_EVENT_INVALID_PACKET_RECEIVED);
    }
}
