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

#include <string.h>
#include <openssl/engine.h>

#include "trustx_engine_common.h"
#include "trustx_engine_ec.h"
#include "trustx.h"

//Globe
trustx_ctx_t trustx_ctx;

//Local
static const char *engine_id   = "trustx_engine";
static const char *engine_name = "Infineon OPTIGA Trust X engine";

static int engine_init(ENGINE *e);
static int engine_finish(ENGINE *e);
static int engine_destroy(ENGINE *e);

static int engine_init(ENGINE *e)
{
  int ret = TRUSTX_ENGINE_FAIL;
  static int initialized = 0;

  do{
    if (initialized) {
	TRUSTX_ENGINE_DBG("Already initialized\n");
	ret = TRUSTX_ENGINE_SUCCESS;
	break;
    }
	  
    TRUSTX_ENGINE_DBGFN("> Engine 0x%x init", (unsigned int) e);
    
    //Init Trustx context
    //trustx_ctx.key[0] = '\0';
    trustx_ctx.ec_key_method = NULL;
    trustx_ctx.ec_key_curve = OPTIGA_ECC_NIST_P_256;
    trustx_ctx.ec_key_usage = OPTIGA_KEY_USAGE_AUTHENTICATION;
    trustx_ctx.key_oid = 0;
    trustx_ctx.pubkeyfilename[0] = '\0';


    //Open trustx application
    if (trustX_Open() != OPTIGA_LIB_SUCCESS)
    {
	TRUSTX_ENGINE_ERRFN("< failed to open OPTIGA application.\n");
	break;
    }
    
    //Init EC
    ret = trustxEngine_init_ec(e);
    if (ret != TRUSTX_ENGINE_SUCCESS) {
	TRUSTX_ENGINE_ERRFN("Engine context init failed");
	break;
    }
    
    //Init Rand
    ret = trustxEngine_init_rand(e);
    if (ret != TRUSTX_ENGINE_SUCCESS) {
	TRUSTX_ENGINE_ERRFN("Init Rand Fail!!");
	break;
    }
    
    ret = TRUSTX_ENGINE_SUCCESS;
    initialized = 1;
  }while(FALSE);
  
  TRUSTX_ENGINE_DBGFN("<");
  return ret;
}

static int engine_destroy(ENGINE *e)
{
  TRUSTX_ENGINE_DBGFN("> Engine 0x%x destroy", (unsigned int) e);
  trustX_Close();
  TRUSTX_ENGINE_DBGFN("<");
  return TRUSTX_ENGINE_SUCCESS;
}

static int engine_finish(ENGINE *e)
{
  TRUSTX_ENGINE_DBGFN("> Engine 0x%x finish (releasing functional reference)", (unsigned int) e);
  TRUSTX_ENGINE_DBGFN("<");
  return TRUSTX_ENGINE_SUCCESS;
}

static int bind(ENGINE *e, const char *id)
{
	int ret = TRUSTX_ENGINE_FAIL;
	
    TRUSTX_ENGINE_DBGFN(">");

	do {
		if (!ENGINE_set_id(e, engine_id)) {
			TRUSTX_ENGINE_DBGFN("ENGINE_set_id failed\n");
			break;
		}
		if (!ENGINE_set_name(e, engine_name)) {
			TRUSTX_ENGINE_DBGFN("ENGINE_set_name failed\n");
			break;
		}

		/* The init function is not allways called so we initialize crypto methods
		   directly from bind. */
		if (!engine_init(e)) {
			TRUSTX_ENGINE_DBGFN("TrustX enigne initialization failed\n");
			break;
		}

		if (!ENGINE_set_load_privkey_function(e, trustx_loadKey)) {
			TRUSTX_ENGINE_DBGFN("ENGINE_set_load_privkey_function failed\n");
			break;
		}
		
		if (!ENGINE_set_finish_function(e, engine_finish)) {
			TRUSTX_ENGINE_DBGFN("ENGINE_set_finish_function failed\n");
			break;
		}

		if (!ENGINE_set_destroy_function(e, engine_destroy)) {
			TRUSTX_ENGINE_DBGFN("ENGINE_set_destroy_function failed\n");
			break;
		}

		//just to try ....ret = ENGINE_set_EC(e, engine_destroy);

/*
		if (!ENGINE_set_ctrl_function(e, engine_ctrl)) {
			TRUSTX_ENGINE_DBGFN("ENGINE_set_ctrl_function failed\n");
			break;
		}


		if (!ENGINE_set_cmd_defns(e, engine_cmd_defns)) {
			TRUSTX_ENGINE_DBGFN("ENGINE_set_cmd_defns failed\n");
			break;
		}
*/
		ret = TRUSTX_ENGINE_SUCCESS;
	}while(FALSE);

    TRUSTX_ENGINE_DBGFN("<");
    return ret;
  }

IMPLEMENT_DYNAMIC_BIND_FN(bind)
IMPLEMENT_DYNAMIC_CHECK_FN()
