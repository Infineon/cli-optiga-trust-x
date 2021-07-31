/**
* MIT License
*
* Copyright (c) 2019 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE

*/
#ifndef _TRUSTX_H_
#define _TRUSTX_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

#include "ifx_i2c_config.h"
#include "optiga_util.h"
#include "optiga_comms.h"
#include "optiga_crypt.h"
#include "pal_os_event.h"

//Debug Print
//#define TRUSTX_HELPER_DEBUG =1

#ifdef TRUSTX_HELPER_DEBUG

#define TRUSTX_HELPER_DBG(x, ...)      fprintf(stderr, "%s:%d " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define TRUSTX_HELPER_DBGFN(x, ...)    fprintf(stderr, "%s:%d %s: " x "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRUSTX_HELPER_ERRFN(x, ...)    fprintf(stderr, "Error in %s:%d %s: " x "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#else

#define TRUSTX_HELPER_DBG(x, ...)
#define TRUSTX_HELPER_DBGFN(x, ...)
#define TRUSTX_HELPER_ERRFN(x, ...)    fprintf(stderr, "Error in %s:%d %s: " x "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#endif

//extern
extern char *i2c_if;
extern char dev[];

// ********** typedef
typedef struct _tag_trustX_UID {
	uint8_t	bCimIdentifer;
	uint8_t bPlatformIdentifier;
	uint8_t bModelIdentifier;
	uint8_t wROMCode[2];
	uint8_t rgbChipType[6];
	uint8_t rgbBatchNumber[6];
	uint8_t wChipPositionX[2];
	uint8_t wChipPositionY[2];
	uint8_t dwFirmwareIdentifier[4];
	uint8_t rgbESWBuild[2];
} trustX_UID_t;

typedef union _tag_utrustX_UID {
	uint8_t b[27];
	trustX_UID_t st;
} utrustX_UID_t;

typedef enum _tag_trustX_LifeCycStatus {
	CREATION 	= 0x01,
	INITIALIZATION 	= 0x03,
	OPERATION	= 0x07,
	TERMINATION	= 0x0f
} trustX_eLifeCycStatus_t;


// *********** Extern
// this is not define in the pal_gpio.h
extern void pal_gpio_init(void);
extern optiga_comms_t optiga_comms;
extern host_lib_status_t optiga_comms_status;


// Function Prototype
void optiga_comms_event_handler(void* upper_layer_ctx, host_lib_status_t event);
optiga_lib_status_t trustX_Open(void);
optiga_lib_status_t trustX_readUID(utrustX_UID_t *UID);
optiga_lib_status_t trustX_readCert(uint16_t oid, uint8_t* p_cert, uint32_t* length);

void trustX_Close(void);

void trustXHexDump(uint8_t *pdata, uint32_t len);
uint16_t trustXWritePEM(uint8_t *buf, uint32_t len, const char *filename, char *name);
uint16_t trustXWriteDER(uint8_t *buf, uint32_t len, const char *filename);
uint16_t trustXReadPEM(uint8_t *buf, uint32_t *len, const char *filename, char *name);
uint16_t trustXReadDER(uint8_t *buf, uint32_t *len, const char *filename);
void trustXdecodeMetaData(uint8_t * metaData);
uint16_t trustXWriteX509PEM(X509 *x509, const char *filename);
uint16_t trustXReadX509PEM(X509 **x509, const char *filename);

#endif	// _TRUSTCX_H_
