/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
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
#include "SceAdc.h"

#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* SCE Header files */
#include "sce/scif.h"
#include "sce/scif_framework.h"
#include "sce/scif_osal_tirtos.h"


/***** Variable declarations *****/
static SceAdc_adcCallback adcCallback;


/***** Prototypes *****/
static void ctrlReadyCallback(void);
static void taskAlertCallback(void);


/***** Function definitions *****/
void SceAdc_init(uint32_t samplingTime, uint32_t minReportInterval, uint16_t adcChangeMask) {
    // Initialize the Sensor Controller
    scifOsalInit();
    scifOsalRegisterCtrlReadyCallback(ctrlReadyCallback);
    scifOsalRegisterTaskAlertCallback(taskAlertCallback);
    scifInit(&scifDriverSetup);
    scifStartRtcTicksNow(samplingTime);

    SCIF_ADC_SAMPLE_CFG_T* pCfg = scifGetTaskStruct(SCIF_ADC_SAMPLE_TASK_ID, SCIF_STRUCT_CFG);
    pCfg->changeMask = adcChangeMask;
    //Set minimum report interval in units of samplingTime
    pCfg->minReportInterval = minReportInterval;
}

void SceAdc_setReportInterval(uint32_t minReportInterval, uint16_t adcChangeMask) {
    //Set the repot inteval and min change in the SC config structure
    SCIF_ADC_SAMPLE_CFG_T* pCfg = scifGetTaskStruct(SCIF_ADC_SAMPLE_TASK_ID, SCIF_STRUCT_CFG);
    pCfg->changeMask = adcChangeMask;
    //Set minimum report interval in units of samplingTime
    pCfg->minReportInterval = minReportInterval;
}

void SceAdc_start(void) {
    // Start task
    scifStartTasksNbl((1 <<SCIF_ADC_SAMPLE_TASK_ID));
}

void SceAdc_registerAdcCallback(SceAdc_adcCallback callback) {
    adcCallback = callback;
}

static void ctrlReadyCallback(void) {
    /* Do nothing */
}

static void taskAlertCallback(void) {

    /* Clear the ALERT interrupt source */
    scifClearAlertIntSource();

    /* Only handle the periodic event alert */
    if (scifGetAlertEvents() & (1 << SCIF_ADC_SAMPLE_TASK_ID))
    {

        /* Get the SCE "output" structure */
        SCIF_ADC_SAMPLE_OUTPUT_T* pOutput = scifGetTaskStruct(SCIF_ADC_SAMPLE_TASK_ID, SCIF_STRUCT_OUTPUT);

        /* Send new ADC value to application via callback */
        if (adcCallback)
        {
            adcCallback(pOutput->adcValue);
        }
    }

    /* Acknowledge the alert event */
    scifAckAlertEvents();
}
