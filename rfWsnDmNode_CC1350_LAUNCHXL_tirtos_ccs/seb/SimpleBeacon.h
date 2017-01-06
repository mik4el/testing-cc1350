/**************************************************************************************************
  Filename:       SimpleBeacon.h

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

#ifndef SimpleBeacon_H
#define SimpleBeacon_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stdbool.h>

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * TYPEDEFS
 */
/// \brief Simple Beacon Status and error codes
typedef enum
{
    SimpleBeacon_Status_Success         = 0, ///Success
    SimpleBeacon_Status_Config_Error    = 1, ///Configuration error
    SimpleBeacon_Status_Param_Error     = 2, ///Param error
    SimpleBeacon_Status_Tx_Error        = 5, ///Tx Error
    SimpleBeacon_Status_Rx_Error        = 6, ///Tx Error
} SimpleBeacon_Status;

/// \brief Structure for the BLE Advertisement Packet
typedef struct
{
    uint8_t *deviceAddress;       ///Device Address used used in the
                                  ///rfc_CMD_BLE_ADV_NC_t Rf command
    uint8_t length;               ///Advertisement packet length
    uint8_t *pAdvData;            ///Advertisement data
} SimpleBeacon_Frame;

/// \brief advertisement interval array in 100us units, optimised for iOS.
///        iOS guidlines state 152.5ms 211.25ms 318.75ms 417.5ms 546.25ms
///        760ms 852.5ms 1022.5ms and 1285ms. In this application we can
///        only advertise for 1s as the sub1G packet rate can be as high
///        as every 1s.
extern uint32_t SimpleBeacon_AdvertisementIntervals[];

/// \brief default advertisement interval default 100us units, used when
///        advertisement times exceeds the elements in the
///        SimpleBeacon_AdvertisementIntervals array
#define SimpleBeacon_DefaultAdvertisementInterval 1000

/// \brief number of advertisement packets sent in 1 period.
#define SimpleBeacon_AdvertisementTimes  10

/*********************************************************************
 * FUNCTIONS
 */

//*****************************************************************************
//
//! \brief Initialise the SimpleBeacon module
//!
//! Initialise the SimpleBeacon module.
//!
//! \param multiClient ued to set multiClient rfMode if there will be multiple
//!                    RF driver clients
//!
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
extern SimpleBeacon_Status SimpleBeacon_init(bool multiClient);

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
extern SimpleBeacon_Status SimpleBeacon_getIeeeAddr(uint8_t *ieeeAddr);

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
extern SimpleBeacon_Status SimpleBeacon_close(void);

//*****************************************************************************
//
//! \brief Send a beacon
//!
//! This function transmits a beacon using the RF driver commands
//!
//! \param beaconFrame Beacon to be advertised
//! \param numTxPerChan Number of transmits per channel
//! \param chanMask channel mask of channels to advertise on
//!
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
extern SimpleBeacon_Status SimpleBeacon_sendFrame(SimpleBeacon_Frame beaconFrame, uint32_t numTxPerChan, uint64_t chanMask);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SimpleBeacon_H */
