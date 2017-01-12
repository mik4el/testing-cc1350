/**************************************************************************************************
  Filename:       SEB.c

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

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include "SEB.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

#define EDDYSTONE_FRAME_TYPE_UID                0x00
#define EDDYSTONE_FRAME_TYPE_URL                0x10
#define EDDYSTONE_FRAME_TYPE_TLM                0x20

#define EDDYSTONE_FRAME_OVERHEAD_LEN            8
#define EDDYSTONE_SVC_DATA_OVERHEAD_LEN         3
#define EDDYSTONE_MAX_URL_LEN                   18
#define EDDYSTONE_UUID_FRAME_LEN                19
#define EDDYSTONE_TLM_FRAME_LEN                 14

// # of URL Scheme Prefix types
#define EDDYSTONE_URL_PREFIX_MAX        4
// # of encodable URL words
#define EDDYSTONE_URL_ENCODING_MAX      14

/*********************************************************************
 * TYPEDEFS
 */

// Eddystone UID frame
typedef struct
{
    uint8_t   frameType;      // UID
    int8_t    rangingData;
    uint8_t   namespaceID[10];
    uint8_t   instanceID[6];
    uint8_t   reserved[2];
} SEB_EUID_t;

// Eddystone URL frame
typedef struct
{
    uint8_t   frameType;      // URL | Flags
    int8_t    txPower;
    uint8_t   encodedURL[EDDYSTONE_MAX_URL_LEN];  // the 1st byte is prefix
} SEB_EURL_t;

// Eddystone TLM frame
typedef struct
{
    uint8_t   frameType;      // TLM
    uint8_t   version;        // 0x00 for now
    uint8_t   vBatt[2];       // Battery Voltage, 1mV/bit, Big Endian
    uint8_t   temp[2];        // Temperature. Signed 8.8 fixed point
    uint8_t   advCnt[4];      // Adv count since power-up/reboot
    uint8_t   secCnt[4];      // Time since power-up/reboot
                              // in 0.1 second resolution
} SEB_ETLM_t;

// Eddystone advertisement frame types
typedef union
{
    SEB_EUID_t        uid;
    SEB_EURL_t        url;
    SEB_ETLM_t        tlm;
} SEB_Eddystone_Frame_t;

typedef struct
{
    uint8_t               length1;        // 2
    uint8_t               dataType1;      // for Flags data type (0x01)
    uint8_t               data1;          // for Flags data (0x04)
    uint8_t               length2;        // 3
    uint8_t               dataType2;      // for 16-bit Svc UUID list data type (0x03)
    uint8_t               data2;          // for Eddystone UUID LSB (0xAA)
    uint8_t               data3;          // for Eddystone UUID MSB (0xFE)
    uint8_t               length;         // Eddystone service data length
    uint8_t               dataType3;      // for Svc Data data type (0x16)
    uint8_t               data4;          // for Eddystone UUID LSB (0xAA)
    uint8_t               data5;          // for Eddystone UUID MSB (0xFE)
    SEB_Eddystone_Frame_t  frame;
} SEB_EddystoneAdvData_t;


/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Array of URL Scheme Prefix's
static char* eddystoneURLPrefix[EDDYSTONE_URL_PREFIX_MAX] =
{
    "http://www.",
    "https://www.",
    "http://",
    "https://"
};

// Array of URLs to be encoded
static char* eddystoneURLEncoding[EDDYSTONE_URL_ENCODING_MAX] =
{
    ".com/",
    ".org/",
    ".edu/",
    ".net/",
    ".info/",
    ".biz/",
    ".gov/",
    ".com/",
    ".org/",
    ".edu/",
    ".net/",
    ".info/",
    ".biz/",
    ".gov/"
};

static uint32_t advCount = 0;

static SEB_EddystoneAdvData_t eddystoneAdv =
{
    0x02,   // length1 2
    1,      // Flags data type
    6,      // Flags data

    // Complete list of 16-bit Service UUIDs
    0x03,   // length of this data including the data type byte
    3,      // 16-bit Svc UUID list data type (0x03)
    0xaa,   // Eddystone UUID LSB (0xAA)
    0xfe,   // Eddystone UUID MSB (0xFE)

    // Service Data
    0x03,   // Eddystone service data length
    0x16,   // Svc Data data type (0x16)
    0xaa,   // Eddystone UUID LSB (0xAA)
    0xfe    // Eddystone UUID MSB (0xFE)

    //Eddysone Frame to be copied in dynamically
};

static SEB_EUID_t eUidFrame;
static SEB_EURL_t eUrlFrame;
static SEB_ETLM_t eTlmFrame;

static uint8_t urlFrameLen;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
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
SimpleBeacon_Status SEB_init(bool multiClient)
{
    return SimpleBeacon_init(multiClient);
}

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
SimpleBeacon_Status SEB_initUID(uint8_t *uidNameSpace, uint8_t *uidInstanceId, int8_t txPower)
{
    //Set frame type
    eUidFrame.frameType = EDDYSTONE_FRAME_TYPE_UID;

    eUidFrame.rangingData = txPower;

    // Set Eddystone UID namespace and instance
    memcpy(eUidFrame.namespaceID, uidNameSpace, 10);
    memcpy(eUidFrame.instanceID, uidInstanceId, 6);

    return SimpleBeacon_Status_Success;
}

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
SimpleBeacon_Status SEB_initUrl(char* urlOrg, int8_t txPower)
{
    uint8_t i, j;
    uint8_t urlLen;
    uint8_t tokenLen;
    uint32_t frameSize;

    //Set length
    urlFrameLen = EDDYSTONE_SVC_DATA_OVERHEAD_LEN;

    // Fill frame with 0s first
    memset((uint8_t*) &eUrlFrame, 0x00, sizeof(SEB_EURL_t));
    //Set frame type
    eUrlFrame.frameType = EDDYSTONE_FRAME_TYPE_URL;

    //set Tx Power
    eUrlFrame.txPower = txPower;

    //Copy in the prefix code and url
    urlLen = (uint8_t) strlen(urlOrg);


    // search for a matching prefix
    for (i = 0; i < EDDYSTONE_URL_PREFIX_MAX; i++)
    {
        tokenLen = strlen(eddystoneURLPrefix[i]);
        if (strncmp(eddystoneURLPrefix[i], urlOrg, tokenLen) == 0)
        {
            break;
        }
    }

    if (i == EDDYSTONE_URL_PREFIX_MAX)
    {
        return SimpleBeacon_Status_Param_Error;       // wrong prefix
    }

    // use the matching prefix number
    eUrlFrame.encodedURL[0] = i;
    urlOrg += tokenLen;
    urlLen -= tokenLen;

    // search for a token to be encoded
    for (i = 0; i < urlLen; i++)
    {
        for (j = 0; j < EDDYSTONE_URL_ENCODING_MAX; j++)
        {
            tokenLen = strlen(eddystoneURLEncoding[j]);
            if (strncmp(eddystoneURLEncoding[j], urlOrg + i, tokenLen) == 0)
            {
                // matching part found
                break;
            }
        }

        if (j < EDDYSTONE_URL_ENCODING_MAX)
        {
            memcpy(&eUrlFrame.encodedURL[1], urlOrg, i);
            // use the encoded byte
            eUrlFrame.encodedURL[i + 1] = j;
            break;
        }
    }

    if (i < urlLen)
    {
        memcpy(&eUrlFrame.encodedURL[i + 2],
               urlOrg + i + tokenLen, urlLen - i - tokenLen);

        frameSize = sizeof(SEB_EURL_t) - EDDYSTONE_MAX_URL_LEN + urlLen - tokenLen + 2;
        urlFrameLen += frameSize;

        return SimpleBeacon_Status_Success;
    }

    frameSize = sizeof(SEB_EURL_t) - EDDYSTONE_MAX_URL_LEN + urlLen;
    urlFrameLen += frameSize;

    memcpy(&eUrlFrame.encodedURL[1], urlOrg, urlLen + 1);

    return SimpleBeacon_Status_Success;
}

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
//! \return SimpleBeacon_Status
//!
//*****************************************************************************
SimpleBeacon_Status SEB_initTLM(uint16_t batt, uint16_t temp, uint32_t time100MiliSec)
{
    //Set frame type
    eTlmFrame.frameType = EDDYSTONE_FRAME_TYPE_TLM;

    eTlmFrame.version = 0;

    // Battery voltage (bit 10:8 - integer, but 7:0 fraction)
    batt = (batt * 125) >> 5; // convert V to mV
    eTlmFrame.vBatt[0] = (batt & 0xFF00) >> 8;
    eTlmFrame.vBatt[1] = batt & 0xFF;
    // Temperature
    eTlmFrame.temp[0] = (temp & 0xFF00) >> 8;;
    eTlmFrame.temp[1] = temp & 0xFF;
    // advertise packet cnt;
    eTlmFrame.advCnt[0] = (advCount & 0xFF000000) >> 24;
    eTlmFrame.advCnt[1] = (advCount & 0x00FF0000) >> 16;
    eTlmFrame.advCnt[2] = (advCount & 0x0000FF00) >> 8;
    eTlmFrame.advCnt[3] = advCount++ & 0xFF;
    // running time
    //time100MiliSec = UTC_getClock() * 10; // 1-second resolution for now
    eTlmFrame.secCnt[0] = (time100MiliSec & 0xFF000000) >> 24;
    eTlmFrame.secCnt[1] = (time100MiliSec & 0x00FF0000) >> 16;
    eTlmFrame.secCnt[2] = (time100MiliSec & 0x0000FF00) >> 8;
    eTlmFrame.secCnt[3] = time100MiliSec & 0xFF;

    return SimpleBeacon_Status_Success;
}

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
SimpleBeacon_Status SEB_sendFrame(SEB_FrameType type, uint8_t* deviceAddress, uint32_t numTxPerChan, uint64_t chanMask)
{
    SimpleBeacon_Frame beaconFrame;
    SimpleBeacon_Status status = SimpleBeacon_Status_Success;

    switch(type)
    {
    case SEB_FrameType_Uuid:
        //Set length
        eddystoneAdv.length = EDDYSTONE_SVC_DATA_OVERHEAD_LEN + EDDYSTONE_UUID_FRAME_LEN;
        //Copy in data
        memcpy(&eddystoneAdv.frame, &eUidFrame, sizeof(SEB_EUID_t));

        beaconFrame.length = EDDYSTONE_FRAME_OVERHEAD_LEN + eddystoneAdv.length;
        beaconFrame.pAdvData = (uint8_t*) &eddystoneAdv;

        break;
    case SEB_FrameType_Url:
        //Set length
        eddystoneAdv.length = urlFrameLen;
        //Copy in data
        memcpy(&eddystoneAdv.frame, &eUrlFrame, sizeof(SEB_EURL_t));

        beaconFrame.length = EDDYSTONE_FRAME_OVERHEAD_LEN + eddystoneAdv.length;
        beaconFrame.pAdvData = (uint8_t*) &eddystoneAdv;
        break;
    case SEB_FrameType_Tlm:
        //Set length
        eddystoneAdv.length = EDDYSTONE_SVC_DATA_OVERHEAD_LEN + EDDYSTONE_TLM_FRAME_LEN;
        //Copy in data
        memcpy(&eddystoneAdv.frame, &eTlmFrame, sizeof(SEB_ETLM_t));

        beaconFrame.length = EDDYSTONE_FRAME_OVERHEAD_LEN + eddystoneAdv.length;
        beaconFrame.pAdvData = (uint8_t*) &eddystoneAdv;
        break;
    }

    //set the device address used in the rfc_CMD_BLE_ADV_NC_t command
    beaconFrame.deviceAddress = deviceAddress;

    //needed until multi client RF driver is supported
    //SimpleBeacon_init();

    status = SimpleBeacon_sendFrame(beaconFrame, numTxPerChan, chanMask);

    //needed until multi client RF driver is supported
    //SimpleBeacon_close();

    return status;
}

/*********************************************************************
*********************************************************************/
