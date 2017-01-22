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

#ifndef TASKS_DMNODERADIOTASKTASK_H_
#define TASKS_DmNODERADIOTASKTASK_H_

#include "stdint.h"
#include "stdbool.h"

#define NODE_SUB1_ACTIVITY_LED Board_PIN_LED0
#define NODE_BLE_ACTIVITY_LED Board_PIN_LED1
#define NODE_ADVERTISE_INVALID  0x00

enum NodeRadioOperationStatus {
    NodeRadioStatus_Success,
    NodeRadioStatus_Failed,
    NodeRadioStatus_FailedNotConnected,
};

typedef enum
{
    Node_AdvertiserNone =     0, //None
    Node_AdvertiserUrl =      1, //Eddystone interleaved URL and TLM
    Node_AdvertiserTypeEnd =  2, //End of advertisement type enum's
} Node_AdvertiserType;

/* Initializes the NodeRadioTask and creates all TI-RTOS objects */
void NodeRadioTask_init(void);

/* Sends an ADC value to the concentrator */
enum NodeRadioOperationStatus NodeRadioTask_sendAdcData(uint16_t data);

/* Sends a BLE beacon with latest data */
void NodeRadioTask_toggleBLE();

/* Get node address, return 0 if node address has not been set */
uint8_t nodeRadioTask_getNodeAddr(void);

/* Convert adc value to double */
double convertADCToTempDouble(uint16_t adcValue);

#define FRACT_BITS 8
#define FIXED2DOUBLE(x) (((double)(x)) / (1 << FRACT_BITS))
#define FLOAT2FIXED(x) ((int)((x) * (1 << FRACT_BITS)))
#define INT2FIXED(x) ((x) << FRACT_BITS)

#endif /* TASKS_DMNODERADIOTASKTASK_H_ */
