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
#ifndef _TRUSTX_ENGINE_COMMON_H_
#define _TRUSTX_ENGINE_COMMON_H_

#include <stdio.h>
#include <openssl/engine.h>

#include "optiga_crypt.h"


// SETTINGS
#define OBJ_MAX_LEN          (128) /* Maximum length for key object paths or passwords */
#define KEY_CONTEXT_MAX_LEN  (100)
#define PARAM_MAX_LEN        (128)

//#define TRUSTX_ENGINE_DEBUG = 1

#ifdef TRUSTX_ENGINE_DEBUG

#define TRUSTX_ENGINE_DBG(x, ...)      fprintf(stderr, "%s:%d " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define TRUSTX_ENGINE_DBGFN(x, ...)    fprintf(stderr, "%s:%d %s: " x "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRUSTX_ENGINE_ERRFN(x, ...)    fprintf(stderr, "Error in %s:%d %s: " x "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRUSTX_ENGINE_MSGFN(x, ...)    fprintf(stderr, "Message: %s:%d %s: " x "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#else

#define TRUSTX_ENGINE_DBG(x, ...)
#define TRUSTX_ENGINE_DBGFN(x, ...)
#define TRUSTX_ENGINE_ERRFN(x, ...)    fprintf(stderr, "Error in %s:%d %s: " x "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRUSTX_ENGINE_MSGFN(x, ...)    fprintf(stderr, "Message: %s:%d %s: " x "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#endif

//Macro define
/// Definition for false
#ifndef FALSE
#define FALSE               (0U)
#endif

/// Definition for true
#ifndef TRUE
#define TRUE                (1U)
#endif

// trustm engine return code
#define TRUSTX_ENGINE_SUCCESS	1
#define TRUSTX_ENGINE_FAIL		0

/*
 * OpenSSL functions typically return 1 on success (as seen in TrouSerS).
 * EVP probably means "enveloped" (Stack Overflow).
 */
#define EVP_SUCCESS ( 1)
#define EVP_FAIL    (-1)

#define PUBKEYFILE_SIZE 256
#define PUBKEY_SIZE 1024

//typedefine
/*
typedef struct trustx_ctx_str
{
  char              key[KEY_CONTEXT_MAX_LEN];
  uint16_t			key_oid;
  char				pubkeyfilename[256];
  EC_KEY_METHOD     *ec_key_method;
} trustx_ctx_t;
*/

typedef enum trustxEngine_flag
{
    TRUSTX_ENGINE_FLAG_NONE = 0x00,
    TRUSTX_ENGINE_FLAG_NEW = 0x01,
    TRUSTX_ENGINE_FLAG_SAVEPUBKEY = 0x02,
    TRUSTX_ENGINE_FLAG_LOCK = 0x80
} trustxEngine_flag_t;

typedef struct trustx_ctx_str
{
  //char      key[KEY_CONTEXT_MAX_LEN];
  uint16_t	key_oid;
  optiga_key_usage_t  ec_key_usage;
  optiga_ecc_curve_t  ec_key_curve;
  EC_KEY_METHOD     *ec_key_method;
  trustxEngine_flag_t ec_flag;
  char			pubkeyfilename[PUBKEYFILE_SIZE];
  uint8_t   pubkey[PUBKEY_SIZE];
  uint16_t  pubkeylen;
  
} trustx_ctx_t;

//extern
extern trustx_ctx_t trustx_ctx;

//function prototype
uint16_t trustxEngine_init_ec(ENGINE *e);
uint16_t trustxEngine_init_rand(ENGINE *e);

#endif // _TRUSTX_ENGINE_COMMON_
