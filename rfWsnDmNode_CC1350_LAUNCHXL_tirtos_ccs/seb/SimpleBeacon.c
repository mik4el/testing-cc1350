/**************************************************************************************************
  Filename:       SimpleBeacon.c

  Description:    This file contains the Simple Beacon API. This module is a wrapper around the RF
                  driver API's needed to send a non connectable advertisements.


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
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <ti/sysbios/knl/Task.h>

// DriverLib
#include <ti/drivers/rf/RF.h>
/*
#include <driverlib/rf_ble_cmd.h>
#include <rf_patches/rf_patch_cpe_ble.h>
#include <rf_patches/rf_overrides_ble.c>
#include <ti/drivers/rf/RF.h>
 */

#include "SimpleBeacon.h"
#include "smartrf_settings/smartrf_settings_ble.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define BLE_PRI_IEEE_ADDR_LOCATION   0x500012E8
#define BLE_SEC_IEEE_ADDR_LOCATION   0x500012D0

#define RF_MODE_MULTIPLE       0x05

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */
/// \brief advertisement interval array in 100us units, optimised for iOS.
///        iOS guidlines state 152.5ms 211.25ms 318.75ms 417.5ms 546.25ms
///        760ms 852.5ms 1022.5ms and 1285ms. In this application we can
///        only advertise for 1s as the sub1G packet rate can be as high
///        as every 1s.
uint32_t SimpleBeacon_AdvertisementIntervals[] = {225, 500, 800, 587, 1075, 988, 1287, 2137, 925};

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static RF_Object bleRfObject;
static RF_Handle bleRfHandle;

static bool configured = false;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

//*****************************************************************************
//
//! \brief Initialise the SimpleBeacon module
//!
//! Initialise the SimpleBeacon module.
//!
//!
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
SimpleBeacon_Status SimpleBeacon_init(bool multiClient)
{
    if(multiClient)
    {
        //set mode to multimode
        RF_pModeBle->rfMode = RF_MODE_MULTIPLE;
    }

    if(!configured)
    {
        bleRfHandle = RF_open(&bleRfObject, RF_pModeBle,
                        (RF_RadioSetup*)RF_ble_pCmdRadioSetup, NULL);

        if(bleRfHandle < 0)
        {
            return SimpleBeacon_Status_Config_Error;
        }

        configured = true;
    }

    return SimpleBeacon_Status_Success;
}

//*****************************************************************************
//
//! \brief Send a beacon
//!
//! This transmits a beacon.
//!
//! \param beaconFrame Beacon to be Tx'ed
//! \param type The frame type to be transmitted
//! \param numTxPerChan Number of transmits per channel, if 0 use default
//! \param chanMask channel mask of channels to Tx on
//!
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
SimpleBeacon_Status SimpleBeacon_sendFrame(SimpleBeacon_Frame beaconFrame, uint32_t numTxPerChan, uint64_t chanMask)
{
    RF_EventMask result;
    SimpleBeacon_Status status = SimpleBeacon_Status_Success;
    uint32_t txCnt;
    uint32_t chIdx;
    rfc_bleAdvOutput_t advStats = {0};

    if(!configured)
    {
        return SimpleBeacon_Status_Config_Error;
    }

    if(numTxPerChan == 0)
    {
        numTxPerChan = SimpleBeacon_AdvertisementTimes;
    }

    RF_ble_pCmdBleAdvNc->startTrigger.triggerType = TRIG_NOW;
    RF_ble_pCmdBleAdvNc->startTrigger.pastTrig = 1;
    RF_ble_pCmdBleAdvNc->startTime = 0;

    RF_ble_pCmdBleAdvNc->pParams->advLen = beaconFrame.length;

    RF_ble_pCmdBleAdvNc->pParams->pAdvData = (uint8_t*) beaconFrame.pAdvData;
    RF_ble_pCmdBleAdvNc->pParams->pDeviceAddress = (uint16_t*) beaconFrame.deviceAddress;


    //only useful for debug
    RF_ble_pCmdBleAdvNc->pOutput = &advStats;

    result = RF_EventLastCmdDone;

    //transmit BLE adv on every channel for number of times specified, or
    //until error
    for (chIdx = 0; ((chIdx < 40) && (result & RF_EventLastCmdDone));
            chIdx++)
    {
        for (txCnt = 0;
            ((txCnt < numTxPerChan) &&
            (result == RF_EventLastCmdDone));
            txCnt++)
        {
            if (chanMask & ((uint64_t)1<<chIdx))
            {
                RF_ble_pCmdBleAdvNc->channel = chIdx;
                RF_ble_pCmdBleAdvNc->whitening.init = 0x40 + chIdx;

                result = RF_runCmd(bleRfHandle, (RF_Op*)RF_ble_pCmdBleAdvNc,
                        RF_PriorityNormal, 0, 0);

                if (!(result & RF_EventLastCmdDone))
                {
                    status = SimpleBeacon_Status_Tx_Error;
                }

                //sleep on all but last advertisement
                if (txCnt+1 < numTxPerChan)
                {
                    uint32_t sleepTime = SimpleBeacon_DefaultAdvertisementInterval;

                    if(txCnt < SimpleBeacon_AdvertisementTimes)
                    {
                        sleepTime = SimpleBeacon_AdvertisementIntervals[txCnt];
                    }

                    Task_sleep(sleepTime);
                }
            }
        }
    }

    if (!(result & RF_EventLastCmdDone))
    {
        status = SimpleBeacon_Status_Config_Error;
    }

    return status;
}

//*****************************************************************************
//
//! \brief Gets the IEEE address
//!
//! This function gets the IEEE address
//!
//! \param ieeeAddr pointer to a 6 element byte array to write the IEEE
//! address to.
//!
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
SimpleBeacon_Status SimpleBeacon_getIeeeAddr(uint8_t *ieeeAddr)
{
    SimpleBeacon_Status status = SimpleBeacon_Status_Param_Error;

    if (ieeeAddr != NULL)
    {
        int i;

        //Reading from primary IEEE location...
        uint8_t *location = (uint8_t *)BLE_PRI_IEEE_ADDR_LOCATION;

        /*
         * ...unless we can find a byte != 0xFF in secondary
         *
         * Intentionally checking all 6 bytes here instead of len, because we
         * are checking validity of the entire IEEE address irrespective of the
         * actual number of bytes the caller wants to copy over.
         */
        for (i = 0; i < 6; i++) {
            if (((uint8_t *)BLE_SEC_IEEE_ADDR_LOCATION)[i] != 0xFF) {
                //A byte in the secondary location is not 0xFF. Use the
                //secondary
                location = (uint8_t *)BLE_SEC_IEEE_ADDR_LOCATION;
                break;
            }
        }

        //inverting byte order
       for (i = 0; i < 6; i++) {
           ieeeAddr[i] = location[6 - 1 - i];
       }


        status = SimpleBeacon_Status_Success;
    }

    return status;
}


//*****************************************************************************
//
//! \brief Closes the SimpleBeacon module
//!
//! Closes the SimpleBeacon module.
//!
//!
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
SimpleBeacon_Status SimpleBeacon_close(void)
{
    if(!configured)
    {
        return SimpleBeacon_Status_Config_Error;
    }

    RF_close(bleRfHandle);

    configured = false;

    return SimpleBeacon_Status_Success;
}

/*********************************************************************
*********************************************************************/
