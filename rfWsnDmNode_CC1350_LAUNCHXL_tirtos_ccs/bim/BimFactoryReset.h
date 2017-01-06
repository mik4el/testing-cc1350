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

#ifndef bimFactoryReset__include
#define bimFactoryReset__include

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USE_BIM

/*******************************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * @fn      BimFactoryReset_hasImage
 *
 * @brief   Determine if the SensorTag has a pre-programmed factory image
 *          in external flash. Criteria for deciding if a factory image is
 *          a sanity check on the vector table and the first instruction of
 *          the executable.
 *
 * @return  none
 */
extern bool BimFactoryReset_hasImage(void);

/*******************************************************************************
 * @fn      BimFactoryReset_applyFactoryImage
 *
 * @brief   Load the factory image from external flash and reboot
 *
 * @return  none
 */
extern void BimFactoryReset_applyFactoryImage(void);

/*******************************************************************************
 * @fn      BimFactoryReset_storeCurrentImage
 *
 * @brief   Save the current image to external flash as factory image
 *
 * @return  none
 */
extern bool BimFactoryReset_storeCurrentImage(void);

/*******************************************************************************
 * @fn      BimFactoryReset_extFlashErase
 *
 * @brief   Erase the external flash
 *
 * @return  none
 */
extern void BimFactoryReset_extFlashErase(void);

#endif // USE_BIM

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* bimFactoryReset__include */
