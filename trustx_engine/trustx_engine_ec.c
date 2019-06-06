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
#include "trustx_engine_ec.h"
#include "trustx_engine_common.h"
#include "pal_os_event.h"

#include "trustx.h"

#include <string.h>

#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/engine.h>

//Globe
const EC_KEY_METHOD *default_ec = NULL;
EC_KEY_METHOD *ec_methods = NULL;

unsigned char dummy_ec_public_key_256[] = 
{
0x30,0x59,0x30,0x13,0x06,0x07,0x2A,0x86,0x48,0xCE,
0x3D,0x02,0x01,0x06,0x08,0x2A,0x86,0x48,0xCE,0x3D,
0x03,0x01,0x07,0x03,0x42,0x00,0x04,0x01,0xA0,0x07,
0x7B,0xB3,0x69,0xFD,0x88,0xD5,0x48,0xB7,0x98,0xBD,
0x42,0xA2,0xF2,0x83,0xAD,0x19,0x31,0xDE,0x83,0x82,
0xE8,0xA7,0xF7,0x6F,0xB0,0x01,0x95,0x35,0xE4,0xFD,
0xBD,0x45,0x79,0x01,0xCC,0xAF,0x2F,0xF1,0x8C,0xBF,
0x0E,0x18,0xE6,0x43,0x3D,0xB8,0x1D,0xFB,0xB1,0x04,
0xAB,0x29,0x50,0x71,0x50,0x16,0x34,0x7A,0x04,0xF3,
0x1F
};

unsigned char dummy_ec_public_key_384[] = 
{
0x30,0x76,0x30,0x10,0x06,0x07,0x2A,0x86,0x48,0xCE,
0x3D,0x02,0x01,0x06,0x05,0x2B,0x81,0x04,0x00,0x22,
0x03,0x62,0x00,0x04,0xF8,0xBF,0xC2,0xFA,0xDF,0xBD,
0xC7,0x2D,0xA1,0x95,0x5C,0xCD,0xF3,0x4B,0x89,0x25,
0x7F,0xC8,0x09,0x2D,0xD6,0xDA,0xE1,0x6C,0xE8,0x45,
0xDD,0x8B,0x61,0x24,0x20,0xEE,0x64,0x9B,0xE0,0x4B,
0xA1,0xD6,0x1A,0x80,0xC3,0xB1,0xCE,0x6F,0xB0,0xB9,
0xC9,0x65,0x21,0x38,0x73,0xE8,0x50,0xBC,0xAB,0x9D,
0xB2,0x21,0xCA,0xE9,0xBB,0x5A,0xD6,0x2C,0xB4,0xDC,
0x2F,0x02,0xB2,0x57,0x52,0xD0,0x43,0xF5,0xB8,0xDD,
0x6B,0x54,0x38,0xCA,0x95,0x46,0x15,0x10,0xEB,0x9E,
0xE3,0xFE,0x89,0xB3,0x93,0x58,0x60,0xDC,0x0A,0xD5
};


static uint32_t parseKeyParams(const char *aArg)
{
	uint32_t value;
	char in[1024];

	char *token[6];
	int   i;
	FILE *fp;
	  
	TRUSTX_ENGINE_DBGFN(">");
    
        strncpy(in, aArg,1024);
  
	if (aArg == NULL)
	{
		TRUSTX_ENGINE_ERRFN("No input key parameters present. (key_oid:<pubkeyfile>)");
		return EVP_FAIL;
	}
	
	i = 0;
	token[0] = strtok(in, ":");
	
	if (token[0] == NULL)
	{
	  TRUSTX_ENGINE_ERRFN("Too few parameters in key parameters list. (key_oid:<pubkeyfile>)");
	  return EVP_FAIL;
	} 
	
	while (token[i] != NULL)
	{
		i++;
		token[i] = strtok(NULL, ":");
	}

	if (i > 6)
	{
	  TRUSTX_ENGINE_ERRFN("Too many parameters in key parameters list. (key_oid:<pubkeyfile>)");
	  return EVP_FAIL;
	}
 
		
	if (strncmp(token[0], "0x",2) == 0)
		sscanf(token[0],"%x",&value);
	else
	{
		if(i==1) 
		{
			fp = fopen((const char *)token[0],"r");
			if (!fp)
			{
				TRUSTX_ENGINE_ERRFN("failed to open key file %s\n",token[0]);
				return EVP_FAIL;
			}

			//Read file
			value = 0;
			fread(&value,2,1, fp); 
			fclose(fp);
			TRUSTX_ENGINE_DBGFN("value : %x",value); 
		}
		else		
			value = 0;
	}

	trustx_ctx.key_oid = value;
	TRUSTX_ENGINE_DBGFN("value %x", value);

	
	if ((token[1] != NULL) && (*(token[1]) != '*') && (*(token[1]) != '^'))
	{
		strncpy(trustx_ctx.pubkeyfilename, token[1], PUBKEYFILE_SIZE);
	}
	else
	{
		trustx_ctx.pubkeyfilename[0]='\0';
		if((token[1] != NULL) && (*(token[1]) == '^'))
		  trustx_ctx.ec_flag = TRUSTX_ENGINE_FLAG_SAVEPUBKEY;
	}


	if ((i>2) && (token[2] != NULL))
	{
			if (!strcmp(token[2],"NEW"))
			{
				// Request NEW key generation
				if (((value >= 0xE0F1) && (value <= 0xE0F3)))
				{
					TRUSTX_ENGINE_DBGFN("found NEW\n");
					trustx_ctx.ec_flag |= TRUSTX_ENGINE_FLAG_NEW;
					if ((i>3) && (strncmp(token[3], "0x",2) == 0))
						sscanf(token[3],"%x",&(trustx_ctx.ec_key_curve));
					if ((i>4) && (strncmp(token[4], "0x",2) == 0))
						sscanf(token[4],"%x",&(trustx_ctx.ec_key_usage));
					if ((i>5) && (strcmp(token[5], "LOCK") == 0))
						trustx_ctx.ec_flag |= TRUSTX_ENGINE_FLAG_LOCK;
				}
				
			}
			else
			{
				// No NEW key request
				TRUSTX_ENGINE_DBGFN("No NEW found\n");
			}
	}


	if (((value < 0xE0F0) || (value > 0xE0F3)))
	{
	  TRUSTX_ENGINE_ERRFN("Invalid Key OID");
	  return EVP_FAIL;
	}

	TRUSTX_ENGINE_DBGFN("<");

	return value;
}

void trustx_disarm_timer(void)
{
  TRUSTX_ENGINE_DBGFN(">");
  pal_os_event_disarm();
  TRUSTX_ENGINE_DBGFN("<");
}


/*
 * With command
 * $ openssl pkey -in mykeyid -engine optiga_trust_ex -inform ENGINE -text_pub -noout
 * the "mykeyid" lands in the parameter "key_id".
 */
EVP_PKEY *trustx_loadKey(
  ENGINE      *e,
  const char  *key_id,
  UI_METHOD   *ui,
  void        *cb_data)
{
  EVP_PKEY    *key         = NULL;
  //EC_KEY      *ecKey       = NULL;
  X509        *x509_cert   = NULL;
  uint8_t      cert[1024];                  // TODO: refactor constant
  uint32_t     cert_len    = sizeof(cert);

  optiga_lib_status_t return_status;
  
  optiga_key_id_t optiga_key_id;
  uint16_t key_oid;
  FILE *fp;
  char *name;
  char *header;
  uint8_t *data;
  uint32_t len;
  uint8_t pubkey[150];
  
  uint16_t i;
  uint8_t eccheader256[] = {0x30,0x59, // SEQUENCE
			    0x30,0x13, // SEQUENCE
			    0x06,0x07, // OID:1.2.840.10045.2.1
			    0x2A,0x86,0x48,0xCE,0x3D,0x02,0x01,
			    0x06,0x08, // OID:1.2.840.10045.3.1.7
			    0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x07};
			    
  uint8_t eccheader384[] = {0x30,0x76, // SEQUENCE
					  0x30,0x10, //SEQUENCE
					  0x06,0x07, // OID:1.2.840.10045.2.1
					  0x2A,0x86,0x48,0xCE,0x3D,0x02,0x01,
					  0x06,0x05, // OID:1.3.132.0.34
					  0x2B,0x81,0x04,0x00,0x22};		    

  TRUSTX_ENGINE_DBGFN(">");
  TRUSTX_ENGINE_DBGFN("key_id=<%s>", key_id);
  TRUSTX_ENGINE_DBGFN("cb_data=0x<%x>", (unsigned int) cb_data);

 
  while (1)
  {
	  
	parseKeyParams(key_id);
	
	key_oid = trustx_ctx.key_oid;
	TRUSTX_ENGINE_DBGFN("trustx_ctx.ec_flag      : 0x%.2x",trustx_ctx.ec_flag);
	TRUSTX_ENGINE_DBGFN("trustx_ctx.ec_key_curve : 0x%.2x",trustx_ctx.ec_key_curve);
	TRUSTX_ENGINE_DBGFN("trustx_ctx.ec_key_usage : 0x%.2x",trustx_ctx.ec_key_usage);
	
	TRUSTX_ENGINE_DBGFN("KEY_OID : 0x%.4x",key_oid);
	if (key_oid == 0xE0F0)
	{
		TRUSTX_ENGINE_DBGFN("Using internal Cert");
		cert_len = sizeof(cert);
		return_status = trustX_readCert(eDEVICE_PUBKEY_CERT_IFX, cert, &cert_len);
		if (return_status != OPTIGA_LIB_SUCCESS)
		{
		  TRUSTX_ENGINE_ERRFN("failed to read certificate with public key from OPTIGA");
		  break;
		}
		TRUSTX_ENGINE_DBGFN("Got raw cert from OPTIGA: 0x%x bytes", cert_len);
	  
		const unsigned char *p = cert;
		x509_cert = d2i_X509(NULL, &p, cert_len);
		if (x509_cert == NULL)
		{
		  // TODO/Hack: If certificate does not start with proper tag
		  TRUSTX_ENGINE_DBGFN("applying metadata hack");
		  const unsigned char *p2 = cert + 9;
		  x509_cert = d2i_X509(NULL, &p2, cert_len - 9);
		  if (x509_cert == NULL)
		  {
			TRUSTX_ENGINE_ERRFN("failed to parse certificate data from OPTIGA");
			break;
		  }
		}
		TRUSTX_ENGINE_DBGFN("Parsed X509 from raw cert");
  
		data = trustx_ctx.pubkey;
		key = X509_get_pubkey(x509_cert);
		trustx_ctx.pubkeylen = i2d_PUBKEY(key,&data);
		
		//trustXHexDump(trustx_ctx.pubkey,trustx_ctx.pubkeylen);
		
		if (key == NULL)
		{
		  TRUSTX_ENGINE_ERRFN("failed to extract public key from X509 certificate");
		  break;
		}
		TRUSTX_ENGINE_DBGFN("Extracted public key from cert");
	}
	else
	{
		if (trustx_ctx.pubkeyfilename[0] != '\0')
		{
			TRUSTX_ENGINE_DBGFN("filename : %s\n",trustx_ctx.pubkeyfilename);
			//open 
			fp = fopen((const char *)trustx_ctx.pubkeyfilename,"r");
			if (!fp)
			{
				TRUSTX_ENGINE_ERRFN("failed to open file %s\n",trustx_ctx.pubkeyfilename);
				break;
			}
			PEM_read(fp, &name,&header,&data,(long int *)&len);
			//memcpy(trustx_ctx.pubkey,data,len);
			trustx_ctx.pubkeylen = len;
			for (i=0; i < len ; i++)
			{
			  trustx_ctx.pubkey[i] = *(data+i);
			}
			
			//trustXHexDump(trustx_ctx.pubkey, trustx_ctx.pubkeylen);
			TRUSTX_ENGINE_DBGFN("len: %d",len);
			
			key = d2i_PUBKEY(NULL,(const unsigned char **)&data,len);
			
		}
		else // 
		{
		  if ((trustx_ctx.ec_flag & TRUSTX_ENGINE_FLAG_NEW) == TRUSTX_ENGINE_FLAG_NEW)
		  {
		    TRUSTX_ENGINE_DBGFN("Generating New Key");
		    optiga_key_id = trustx_ctx.key_oid;
		    
		    len = sizeof(pubkey);
		    if(trustx_ctx.ec_key_curve == OPTIGA_ECC_NIST_P_256)
		    {
		      trustx_ctx.pubkeylen = sizeof(eccheader256);
		      for (i=0;i<trustx_ctx.pubkeylen;i++)
		      {
			pubkey[i] = eccheader256[i];
		      }
		    }
		    else
		    {
		      trustx_ctx.pubkeylen = sizeof(eccheader384);
		      for (i=0;i<trustx_ctx.pubkeylen;i++)
		      {
			pubkey[i] = eccheader384[i];
		      }    
		    }
		    
		    return_status = optiga_crypt_ecc_generate_keypair(trustx_ctx.ec_key_curve, //key size 256
								      trustx_ctx.ec_key_usage, // Type = Auth
								      FALSE,
								      &optiga_key_id,
								      (pubkey+i),
								      (uint16_t *)&len);
		    if (return_status != OPTIGA_LIB_SUCCESS)
		    {
		      TRUSTX_ENGINE_ERRFN("Error!!! [0x%.8X]\n",return_status);
		      break;
		    }

		    if ((trustx_ctx.ec_flag & TRUSTX_ENGINE_FLAG_SAVEPUBKEY) == TRUSTX_ENGINE_FLAG_SAVEPUBKEY)
		    {
		      TRUSTX_ENGINE_DBGFN("Save Pubkey to : 0x%.4x",(trustx_ctx.key_oid) + (0xF1D0-0xE0F0));
		      return_status = optiga_util_write_data((trustx_ctx.key_oid) + (0xF1D0-0xE0F0),
								OPTIGA_UTIL_WRITE_ONLY,
								0,
								(pubkey+i), 
								len);

		      if (return_status != OPTIGA_LIB_SUCCESS)
		      {
			TRUSTX_ENGINE_ERRFN("Error!!! [0x%.8X]",return_status);
		      }
		      else
		      {
			TRUSTX_ENGINE_DBGFN("Pubkey Write Success.");
			TRUSTX_ENGINE_DBGFN("Pubkey Len : %d", len);
			
		      }			      
		    }

		    data = pubkey;		
		    memcpy(trustx_ctx.pubkey,data,len+i);
		    trustx_ctx.pubkeylen = len+i;		    
		    key = d2i_PUBKEY(NULL,(const unsigned char **)&data,len+i);
		    

		  }
		  else // Load Dummy Pubkey
		  {
		    if ((trustx_ctx.ec_flag & TRUSTX_ENGINE_FLAG_SAVEPUBKEY) == TRUSTX_ENGINE_FLAG_SAVEPUBKEY)
		    {
		      TRUSTX_ENGINE_DBGFN("Load Pubkey from : 0x%.4x",(trustx_ctx.key_oid) + (0xF1D0-0xE0F0));
		      len = sizeof(pubkey);
		      
	      
		      return_status = optiga_util_read_data((trustx_ctx.key_oid) + (0xF1D0-0xE0F0),
							    0,
							    pubkey,
							    (uint16_t *)&len);

		      if (return_status != OPTIGA_LIB_SUCCESS)
		      {
			printf("Error!!! [0x%.8X]\n",return_status);
		      }
		      else
		      {
			//TRUSTX_ENGINE_DBGFN("Pubkey Load Success.");
			len = pubkey[1]+2;
			data = pubkey;	
			//TRUSTX_ENGINE_DBGFN("len : %d", len);
			if(len == 0x44)
			{
			  trustx_ctx.pubkeylen = sizeof(eccheader256);
			  for (i=0;i<trustx_ctx.pubkeylen;i++)
			  {
			    trustx_ctx.pubkey[i] = eccheader256[i];
			  }
			  trustx_ctx.ec_key_curve = OPTIGA_ECC_NIST_P_256;
			}
			else
			{
			  trustx_ctx.pubkeylen = sizeof(eccheader384);
			  for (i=0;i<trustx_ctx.pubkeylen;i++)
			  {
			    trustx_ctx.pubkey[i] = eccheader384[i];
			  }    
			  trustx_ctx.ec_key_curve = OPTIGA_ECC_NIST_P_384;

			}
			TRUSTX_ENGINE_DBGFN("trustx_ctx.ec_key_curve : 0x%.2x",trustx_ctx.ec_key_curve);	
			memcpy(&trustx_ctx.pubkey[i],data,len);
			trustx_ctx.pubkeylen += len;
			data = trustx_ctx.pubkey;		    
			key = d2i_PUBKEY(NULL,(const unsigned char **)&data,trustx_ctx.pubkeylen);
		      }
		    }
		    else
		    {		  
		      TRUSTX_ENGINE_DBGFN("No Pubkey filename. Load Prikey Only With Dummy public Key\n");
		      data = dummy_ec_public_key_256;
		      trustx_ctx.pubkeylen = 0;		
		      key = d2i_PUBKEY(NULL,(const unsigned char **)&data,sizeof(dummy_ec_public_key_256));
		    }
		  }
		}
	}

    TRUSTX_ENGINE_DBGFN("<");
    trustx_disarm_timer();    
    return key; // SUCCESS
  }

  trustx_disarm_timer();
  TRUSTX_ENGINE_ERRFN("<");
  return (EVP_PKEY *) NULL; // RETURN FAIL
}

static ECDSA_SIG* trustx_ecdsa_sign(
  const unsigned char  *dgst,
  int                   dgstlen,
  const BIGNUM         *in_kinv,
  const BIGNUM         *in_r,
  EC_KEY               *eckey
)
{
  TRUSTX_ENGINE_DBGFN(">");
  TRUSTX_ENGINE_DBGFN("oid : 0x%.4x",trustx_ctx.key_oid);
  TRUSTX_ENGINE_DBGFN("dgst len : %d",dgstlen);

  uint8_t     sig[256];
  uint16_t    sig_len = 256;

  optiga_lib_status_t return_status;
  //int	ret = TRUSTX_ENGINE_FAIL;
  ECDSA_SIG  *ecdsa_sig = NULL;
  
  // TODO/HACK:
  if (dgstlen != 32)
  {
    dgstlen = 32;
    TRUSTX_ENGINE_DBGFN("APPLIED digest length hack");
  }

  do {
    return_status = optiga_crypt_ecdsa_sign((uint8_t *) dgst,
					    dgstlen,
					     trustx_ctx.key_oid,
					     (sig+2), 
					     &sig_len);
    if (return_status != OPTIGA_LIB_SUCCESS)                                             
    {
      TRUSTX_ENGINE_ERRFN("Could not get signature form OPTIGA : %x", return_status);

      trustx_disarm_timer();
      break;
    }

    TRUSTX_ENGINE_DBGFN("Signature received : sig+2=%x, sig_len=0x%x=%d",
      (unsigned int) sig+2,
      sig_len, sig_len);

    sig[0] = 0x30;
    sig[1] = sig_len;
    const unsigned char *p = sig;
    ecdsa_sig = d2i_ECDSA_SIG(NULL, &p, sig_len+2);

  }while(FALSE);
  
  trustx_disarm_timer();
  TRUSTX_ENGINE_DBGFN("<");
  //return ret;
  return ecdsa_sig;
  
}

/*
 * Initializes the global engine context.
 * Return 1 on success, otherwise 0.
 */
uint16_t trustxEngine_init_ec(ENGINE *e)
{
  uint16_t ret = TRUSTX_ENGINE_FAIL;
  int (*orig_sign) (int, const unsigned char *, int, unsigned char *,
                    unsigned int *, const BIGNUM *, const BIGNUM *, EC_KEY *)
        = NULL;

  // Verify method
  int (*orig_verify)(int type,const unsigned char *dgst,int dgst_len,const unsigned char *sigbuf,
		    int sig_len,EC_KEY *eckey) = NULL;
  int (*orig_verify_sig)(const unsigned char *dgst,int dgst_len,const ECDSA_SIG *sig,
		    EC_KEY *eckey) = NULL;

  TRUSTX_ENGINE_DBGFN(">");

  do {
	default_ec = EC_KEY_OpenSSL();
	if (default_ec == NULL)
	  break;

	ec_methods = EC_KEY_METHOD_new(default_ec);
	trustx_ctx.ec_key_method = ec_methods;
	if (ec_methods == NULL)
	  break;

	EC_KEY_METHOD_get_sign(ec_methods, &orig_sign, NULL, NULL);
	EC_KEY_METHOD_set_sign(ec_methods, orig_sign, NULL, trustx_ecdsa_sign);

	// Need to used OpenSSL verify as HW device has limited verification
	EC_KEY_METHOD_get_verify(ec_methods, &orig_verify,&orig_verify_sig);
	EC_KEY_METHOD_set_verify(ec_methods, orig_verify, orig_verify_sig);
	   		
	ret = ENGINE_set_EC(e, ec_methods);
    
      }while(FALSE);

  TRUSTX_ENGINE_DBGFN("<");
  return ret;
}
