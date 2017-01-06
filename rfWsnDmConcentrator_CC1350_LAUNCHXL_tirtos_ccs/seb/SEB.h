/**************************************************************************************************
  Filename:       SEB.h

  Description:    This file contains the Simple Eddystone Beacon formatting.

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

#ifndef SEB_H
#define SEB_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stdbool.h>
#include "SimpleBeacon.h"

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/// \brief BLE Advertisement Status and error codes
typedef enum
{
    SEB_FrameType_Uuid         = 0, ///UUID Frame
    SEB_FrameType_Url          = 1, ///URL Frame
    SEB_FrameType_Tlm          = 2, ///TLM Frame
} SEB_FrameType;


/*********************************************************************
 * FUNCTIONS
 */

//*****************************************************************************
//
//! \brief Initialise Simple Eddystone Beacon Module
//!
//! This function initialises SimpleBeacon module
//!
//! \param multiClient ued to set multiClient rfMode if there will be multiple
//!                    RF driver clients

//! \return SimpleBeacon_Status
//!
//*****************************************************************************
extern SimpleBeacon_Status SEB_init(bool multiClient);

//*****************************************************************************
//
//! \brief Initialise Eddystone UUID Frame
//!
//! This function initialises the UUID frame with predefined UUID.
//!
//! \param uidNameSpace 10 byte uid namespace
//! \param uidInstance 6 byte uid instance id
//! \param txPower Tx Power at 0m
//!
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
extern SimpleBeacon_Status SEB_initUID(uint8_t *uidNameSpace, uint8_t *uidInstanceId, int8_t txPower);

//*****************************************************************************
//
//! \brief Update Eddystone URL Frame
//!
//! This function Updates the URL frame with the url string provided.
//!
//! \param urlOrg URL to be encoded into the UL frame
//! \param txPower Tx Power at 0m
//!
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
extern SimpleBeacon_Status SEB_initUrl(char* urlOrg, int8_t txPower);

//*****************************************************************************
//
//! \brief Update Eddystone TLM Frame
//!
//! This function Updates the TLM frame with the battery level time stamp and
//! temperature provided.
//!
//! \param batt Current Battery voltage (bit 10:8 - integer, but 7:0 fraction)
//! \param temp Current temperature
//! \param time100MiliSec time since boot in 100ms units
//!
//! \return SEB_Status
//!
//*****************************************************************************
extern SimpleBeacon_Status SEB_initTLM(uint16_t batt, uint16_t temp, uint32_t time100MiliSec);

//*****************************************************************************
//
//! \brief Send an Eddystone beacon
//!
//! This transmits and Eddystone beacon.
//!
//! \param type The frame type to be transmitted
//! \param 6 Byte BLE MAC address
//! \param numTxPerChan Number of transmits per channel
//! \param chanMask channel mask of channels to Tx on
//!
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
extern SimpleBeacon_Status SEB_sendFrame(SEB_FrameType type, uint8_t* deviceAddress, uint32_t numTxPerChan, uint64_t chanMask);


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SEB_H */
