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
#include "optiga/optiga_util.h"
#include "optiga_crypt.h"

static int trustxEngine_getrandom(unsigned char *buf, int num);
static int trustxEngine_rand_status(void);

// OpenSSL random method define
static RAND_METHOD rand_methods = {
    NULL,        			// seed()
    trustxEngine_getrandom,
    NULL,        			// cleanup()
    NULL,				// add()
    trustxEngine_getrandom,		// pseudorand()
    trustxEngine_rand_status		// status()
};

/** Return the entropy status of the prng
 * Since we provide real randomness 
 * function, our status is allways good.
 * @retval 1 allways good status
 */
static int trustxEngine_rand_status(void)
{
    return TRUSTX_ENGINE_SUCCESS;
}

/** Initialize the trusttx rand 
 *
 * @param e The engine context.
 */
uint16_t trustxEngine_init_rand(ENGINE *e)
{
	uint16_t ret = TRUSTX_ENGINE_FAIL;
	TRUSTX_ENGINE_DBGFN(">");
	
	ret = ENGINE_set_RAND(e, &rand_methods);
    
	TRUSTX_ENGINE_DBGFN("<");
    return ret;
    
}

/** Genereate random values
 * @param buf The buffer to write the random values to
 * @param num The amound of random bytes to generate
 * @retval 1 on success
 * @retval 0 on failure
 */
static int trustxEngine_getrandom(unsigned char *buf, int num)
{
	#define MAX_RAND_INPUT 256

	int ret = TRUSTX_ENGINE_FAIL;
	optiga_lib_status_t return_status;
	int i,j,k;
	uint8_t tempbuf[MAX_RAND_INPUT];
	
	TRUSTX_ENGINE_DBGFN("> num : %d", num);

	i = num % MAX_RAND_INPUT; // max random number output, find the reminder
	j = (num - i)/MAX_RAND_INPUT; // Get the count 

	do {
		k = 0;
		if(i > 0)  
		{
			// Trustx lib doesn't work with small num. So used Max
			return_status = optiga_crypt_random(OPTIGA_RNG_TYPE_TRNG,
						tempbuf,
						MAX_RAND_INPUT);
			if (return_status != OPTIGA_LIB_SUCCESS)
			{
			  TRUSTX_ENGINE_ERRFN("failed to generate random number2");
			  printf("return error = %x\n",return_status);
			  break;
			}

			for (k=0;k<i;k++)
			{
				*(buf+k) = tempbuf[k]; 
			}
		}

		for(;j>0;j--)  
		{
			return_status = optiga_crypt_random(OPTIGA_RNG_TYPE_TRNG,
						(buf+k),
						MAX_RAND_INPUT);
			if (return_status != OPTIGA_LIB_SUCCESS)
			{
			  TRUSTX_ENGINE_ERRFN("failed to generate random number1");
			  break;
			}
			k += (MAX_RAND_INPUT);
		}

		
		ret = TRUSTX_ENGINE_SUCCESS;
	}while(FALSE);
	
	#undef MAX_RAND_INPUT
	trustx_disarm_timer(); // temp work around for trustx lib
	TRUSTX_ENGINE_DBGFN("<");	
	return ret;
}
