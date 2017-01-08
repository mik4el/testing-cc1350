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
#include <DmConcentratorRadioTask.h>
#include <DmConcentratorTask.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <string.h>

#include <ti/sysbios/BIOS.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>

/* Drivers */
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/display/Display.h>
#include <ti/display/DisplayExt.h>

/* Board Header files */
#include "Board.h"

#include "RadioProtocol.h"



/***** Defines *****/
#define CONCENTRATOR_TASK_STACK_SIZE 1024
#define CONCENTRATOR_TASK_PRIORITY   3
#define CONCENTRATOR_EVENT_ALL                         0xFFFFFFFF
#define CONCENTRATOR_EVENT_NEW_ADC_SENSOR_VALUE    (uint32_t)(1 << 0)
#define CONCENTRATOR_MAX_NODES 7
#define CONCENTRATOR_DISPLAY_LINES 10

/***** Type declarations *****/
struct AdcSensorNode {
    uint8_t address;
    uint16_t latestAdcValue;
    int32_t latestInternalTempValue;
    uint8_t button;
    int8_t latestRssi;
};

/*
 * Application button pin configuration table:
 *   - Buttons interrupts are configured to trigger on falling edge.
 */
PIN_Config buttonPinTable[] = {
    Board_PIN_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
    Board_PIN_BUTTON1  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
    PIN_TERMINATE
};

/***** Variable declarations *****/
static Task_Params concentratorTaskParams;
Task_Struct concentratorTask;    /* not static so you can see in ROV */
static uint8_t concentratorTaskStack[CONCENTRATOR_TASK_STACK_SIZE];
Event_Struct concentratorEvent;  /* not static so you can see in ROV */
static Event_Handle concentratorEventHandle;
static struct AdcSensorNode latestActiveAdcSensorNode;
struct AdcSensorNode knownSensorNodes[CONCENTRATOR_MAX_NODES];
static struct AdcSensorNode* lastAddedSensorNode = knownSensorNodes;
static uint8_t selectedNode = 0;
static Display_Handle hDisplayLcd;
static Display_Handle hDisplaySerial;
static PIN_Handle buttonPinHandle;
static PIN_State buttonPinState;
static ConcentratorAdvertiser advertiser;

/***** Prototypes *****/
static void concentratorTaskFunction(UArg arg0, UArg arg1);
static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi);
static void updateLcd(void);
static void addNewNode(struct AdcSensorNode* node);
static void updateNode(struct AdcSensorNode* node);
static uint8_t isKnownNodeAddress(uint8_t address);
void buttonCallback(PIN_Handle handle, PIN_Id pinId);

/***** Function definitions *****/
void ConcentratorTask_init(void) {

    /* Create event used internally for state changes */
    Event_Params eventParam;
    Event_Params_init(&eventParam);
    Event_construct(&concentratorEvent, &eventParam);
    concentratorEventHandle = Event_handle(&concentratorEvent);

    /* Create the concentrator radio protocol task */
    Task_Params_init(&concentratorTaskParams);
    concentratorTaskParams.stackSize = CONCENTRATOR_TASK_STACK_SIZE;
    concentratorTaskParams.priority = CONCENTRATOR_TASK_PRIORITY;
    concentratorTaskParams.stack = &concentratorTaskStack;
    Task_construct(&concentratorTask, concentratorTaskFunction, &concentratorTaskParams, NULL);
}

static void concentratorTaskFunction(UArg arg0, UArg arg1)
{
    /* Initialize display and try to open both UART and LCD types of display. */
    Display_Params params;
    Display_Params_init(&params);
    params.lineClearMode = DISPLAY_CLEAR_BOTH;

    /* Open both an available LCD display and an UART display.
     * Whether the open call is successful depends on what is present in the
     * Display_config[] array of the board file.
     *
     * Note that for SensorTag evaluation boards combined with the SHARP96x96
     * Watch DevPack, there is a pin conflict with UART such that one must be
     * excluded, and UART is preferred by default. To display on the Watch
     * DevPack, add the precompiler define BOARD_DISPLAY_EXCLUDE_UART.
     */
    hDisplayLcd = Display_open(Display_Type_LCD, &params);
    hDisplaySerial = Display_open(Display_Type_UART, &params);

    /* Check if the selected Display type was found and successfully opened */
    if (hDisplaySerial)
    {
        Display_printf(hDisplaySerial, 0, 0, "Waiting for nodes...");
    }

    /* Check if the selected Display type was found and successfully opened */
    if (hDisplayLcd)
    {
        Display_printf(hDisplayLcd, 0, 0, "Waiting for nodes...");
    }

    buttonPinHandle = PIN_open(&buttonPinState, buttonPinTable);
    if(!buttonPinHandle)
    {
        System_abort("Error initializing button pins\n");
    }

    /* Setup callback for button pins */
    if (PIN_registerIntCb(buttonPinHandle, &buttonCallback) != 0)
    {
        System_abort("Error registering button callback function");
    }

    /* Register a packet received callback with the radio task */
    ConcentratorRadioTask_registerPacketReceivedCallback(packetReceivedCallback);

    /* set defauls for the ble advertiser settings */
    advertiser.sourceAddress = CONCENTRATOR_ADVERTISE_INVALID;
    advertiser.type = Concentrator_AdertiserNone;

    ConcentratorRadioTask_setAdvertiser(advertiser);

    //set selected node to 0
    selectedNode = 0;

    /* Enter main task loop */
    while (1)
    {
        /* Wait for event */
        uint32_t events = Event_pend(concentratorEventHandle, 0, CONCENTRATOR_EVENT_ALL, BIOS_WAIT_FOREVER);

        /* If we got a new ADC sensor value */
        if (events & CONCENTRATOR_EVENT_NEW_ADC_SENSOR_VALUE)
        {
            /* If we knew this node from before, update the value */
            if (isKnownNodeAddress(latestActiveAdcSensorNode.address))
            {
                updateNode(&latestActiveAdcSensorNode);
            }
            else
            {
                /* Else add it */
                addNewNode(&latestActiveAdcSensorNode);
            }

            /* Update the values on the LCD */
            updateLcd();
        }
    }
}

static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi)
{
    if (packet->header.packetType == RADIO_PACKET_TYPE_DM_SENSOR_PACKET)
    {
        /* Save the values */
        latestActiveAdcSensorNode.address = packet->header.sourceAddress;
        latestActiveAdcSensorNode.latestAdcValue = packet->dmSensorPacket.adcValue;
        latestActiveAdcSensorNode.latestInternalTempValue = packet->dmSensorPacket.internalTemp;
        latestActiveAdcSensorNode.button = packet->dmSensorPacket.button;
        latestActiveAdcSensorNode.latestRssi = rssi;

        Event_post(concentratorEventHandle, CONCENTRATOR_EVENT_NEW_ADC_SENSOR_VALUE);
    }
}

static uint8_t isKnownNodeAddress(uint8_t address) {
    uint8_t found = 0;
    uint8_t i;
    for (i = 0; i < CONCENTRATOR_MAX_NODES; i++)
    {
        if (knownSensorNodes[i].address == address)
        {
            found = 1;
            break;
        }
    }
    return found;
}

static void updateNode(struct AdcSensorNode* node) {
    uint8_t i;
    for (i = 0; i < CONCENTRATOR_MAX_NODES; i++) {
        if (knownSensorNodes[i].address == node->address)
        {
            knownSensorNodes[i].latestAdcValue = node->latestAdcValue;
            knownSensorNodes[i].latestInternalTempValue = node->latestInternalTempValue;
            knownSensorNodes[i].latestRssi = node->latestRssi;
            knownSensorNodes[i].button = node->button;
            break;
        }
    }
}

static void addNewNode(struct AdcSensorNode* node) {
    *lastAddedSensorNode = *node;

    /* Increment and wrap */
    lastAddedSensorNode++;
    if (lastAddedSensorNode > &knownSensorNodes[CONCENTRATOR_MAX_NODES-1])
    {
        lastAddedSensorNode = knownSensorNodes;
    }

    if(advertiser.sourceAddress == CONCENTRATOR_ADVERTISE_INVALID)
    {
        /* set first node as advertiser */
        advertiser.sourceAddress = node->address;
        ConcentratorRadioTask_setAdvertiser(advertiser);
    }
}

static void updateLcd(void) {
    struct AdcSensorNode* nodePointer = knownSensorNodes;
    uint8_t currentLcdLine;
    char selectedChar;

    Display_clear(hDisplayLcd);
    Display_printf(hDisplayLcd, 0, 0, "Mik4el");
    Display_printf(hDisplayLcd, 2, 0, "Nodes TempI RSSI");

    //clear screen, put cursor to beginning of terminal and print the header
    Display_printf(hDisplaySerial, 0, 0, "\033[2J \033[0;0HNodes    Value    RSSI");

    /* Start on the fourth line */
    currentLcdLine = 3;

    /* Write one line per node */
    while ((nodePointer < &knownSensorNodes[CONCENTRATOR_MAX_NODES]) &&
          (nodePointer->address != 0) &&
          (currentLcdLine < CONCENTRATOR_DISPLAY_LINES))
    {
        if ( currentLcdLine == (selectedNode + 1))
        {
            selectedChar = '*';
        }
        else
        {
            selectedChar = ' ';
        }
        /* print to LCD */
        Display_printf(hDisplayLcd, currentLcdLine, 0, "%c0x%02x %d  %04d", selectedChar,
                nodePointer->address, nodePointer->latestInternalTempValue, nodePointer->latestRssi);

        /* print to UART */
        Display_printf(hDisplaySerial, 0, 0, "%c0x%02x    %d    %04d", selectedChar,
                nodePointer->address, nodePointer->latestInternalTempValue, nodePointer->latestRssi);

        nodePointer++;
        currentLcdLine++;
    }

    //if we have some nodes print the advertiser mode
    if (currentLcdLine > 1)
    {
        char advMode[16] = {0};

        /* print button status */
        if (knownSensorNodes[selectedNode].button)
        {
            Display_printf(hDisplayLcd, currentLcdLine, 0, "Button Pressed");
            Display_printf(hDisplaySerial, 0, 0, "Button Pressed");
        }
        else
        {
            Display_printf(hDisplayLcd, currentLcdLine, 0, "No Button Press");
            Display_printf(hDisplaySerial, 0, 0, "No Button Pressed");
        }


        if (advertiser.type == Concentrator_AdertiserMsUrl)
        {
             strncpy(advMode, "BLE MS + URL", 12);
        }
        else if (advertiser.type == Concentrator_AdertiserMs)
        {
             strncpy(advMode, "BLE MS", 6);
        }
        else if (advertiser.type == Concentrator_AdertiserUrl)
        {
             strncpy(advMode, "Eddystone URL", 13);
        }
        else if (advertiser.type == Concentrator_AdertiserUid)
        {
             strncpy(advMode, "Eddystone UID", 13);
        }
        else
        {
             strncpy(advMode, "BLE None", 8);
        }

        /* print to LCD */
        //Display_printf(hDisplayLcd, currentLcdLine+1, 0, "Adv Mode:");
        Display_printf(hDisplayLcd, currentLcdLine+1, 0, "%s", advMode);
        /* print to UART */
        Display_printf(hDisplaySerial, 0, 0, "Advertiser Mode: %s", advMode);
    }
}

/*
 *  ======== buttonCallback ========
 *  Pin interrupt Callback function board buttons configured in the pinTable.
 */
void buttonCallback(PIN_Handle handle, PIN_Id pinId)
{
    /* Debounce logic, only toggle if the button is still pushed (low) */
    CPUdelay(8000*50);

    if (PIN_getInputValue(Board_PIN_BUTTON0) == 0)
    {
        //select node
        selectedNode++;
        if ( (selectedNode >CONCENTRATOR_MAX_NODES) ||
             (knownSensorNodes[selectedNode].address == 0) )
        {
            selectedNode = 0;
        }

        advertiser.type = Concentrator_AdertiserNone;
        advertiser.sourceAddress = knownSensorNodes[selectedNode].address;
        ConcentratorRadioTask_setAdvertiser(advertiser);

        //trigger LCD update
        Event_post(concentratorEventHandle, CONCENTRATOR_EVENT_NEW_ADC_SENSOR_VALUE);
    }
    else if (PIN_getInputValue(Board_PIN_BUTTON1) == 0)
    {
        //cycle between ms, url, uid and none
        advertiser.type++;
        if (advertiser.type == Concentrator_AdertiserTypeEnd)
        {
            advertiser.type = Concentrator_AdertiserNone;
        }

        //Set advertiemer
        ConcentratorRadioTask_setAdvertiser(advertiser);

        //trigger LCD update
        Event_post(concentratorEventHandle, CONCENTRATOR_EVENT_NEW_ADC_SENSOR_VALUE);
    }
}
