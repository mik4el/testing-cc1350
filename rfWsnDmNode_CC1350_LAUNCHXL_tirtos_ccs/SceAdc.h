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

#ifndef SCEADC_H_
#define SCEADC_H_

#include "stdint.h"
#include "sce/scif.h"


typedef void(*SceAdc_adcCallback)(uint16_t adcValue);

/* Intializes the SCE ADC sampling task.
 *
 * This loads the SCE with the ADC sampling task, sets the sampling time and the ADC change mask.
 * The ADC change mask can be used to mask the bits that needs to be changed for the ADC task to
 * signal, and potentially wake up, the CM3 with a new sensor value update.
 * The Minimun Report Interval will be used to send sensor data on a minimum interval incase sensor
 * data does not change within this time. The Minimun Report Interval can be set to 0 if no minimum
 * report interval is required.
 *
 * Note that this does not start the task, see SceAdc_start for starting a task.
 */
void SceAdc_init(uint32_t samplingTime, uint32_t minReportInterval, uint16_t adcChangeMask);

/* Sets the SCE ADC sampling task report interval and minimum change.
 *
 * The ADC change mask can be used to mask the bits that needs to be changed for the ADC task to
 * signal, and potentially wake up, the CM3 with a new sensor value update.
 * The Minimun Report Interval will be used to send sensor data on a minimum interval incase sensor
 * data does not change within this time. The Minimun Report Interval can be set to 0 if no minimum
 * report interval is required.
 *
 * Note that this can be called after the task has been started.
 */
void SceAdc_setReportInterval(uint32_t minReportInterval, uint16_t adcChangeMask);

/* Register the callback used for receiving the updated ADC value.
 *
 * Note that only one callback may be registered at a time.
 */
void SceAdc_registerAdcCallback(SceAdc_adcCallback callback);

/* Starts the SCE ADC sampling task.
 *
 * The task has to be initialized using SceAdc_init before being started. */
void SceAdc_start(void);


#endif /* SCEADC_H_ */
