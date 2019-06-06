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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/asn1.h>

#include "optiga/ifx_i2c/ifx_i2c_config.h"
#include "optiga/optiga_util.h"

#include "trustx.h"

//Globe
char *i2c_if;
char dev[]="/dev/i2c-1";

extern void pal_gpio_init(void);
extern void pal_gpio_deinit(void);
extern pal_status_t pal_init(void);
extern pal_status_t pal_os_event_init(void);
extern pal_status_t pal_os_event_stop(void);

/*************************************************************************
*  Global
*************************************************************************/
extern ifx_i2c_context_t ifx_i2c_context_0;
optiga_comms_t optiga_comms = {(void*)&ifx_i2c_context_0, NULL,NULL, OPTIGA_COMMS_SUCCESS};

/*************************************************************************
*  Read Metadata support
************************************************************************/
static char __ALW[] = "ALW";
static char __LCSG[] = "LcsG";
static char __LCSA[] = "LcsA";
static char __LCSO[] = "LcsO";
static char __EQ[] = "==";
static char __GT[] = ">";
static char __LT[] = "<";
static char __AND[] = "&&";
static char __OR[] = "||";
static char __NEV[] = "NEV";
static char __ECC256[] = "ECC256";
static char __ECC384[] = "ECC384";
static char __SHA256[] = "SHA256";
static char __AUTH[] = "Auth";
static char __ENC[] = "Enc";
static char __HFU[] = "HFU";
static char __DM[] = "DM";
static char __SIGN[] = "Sign";
static char __AGREE[] = "Agreement";
static char __E1[100];

static char* __decodeAC(uint8_t data)
{
	char *ret;
	switch (data)
	{
		case 0x00:
			ret = __ALW; 
			break;
		case 0x70:
			ret = __LCSG;
			break;
		case 0xE0:
			ret = __LCSA;
			break;
		case 0xE1:
			ret = __LCSO;
			break;
		case 0xFA:
			ret = __EQ;
			break;
		case 0xFB:
			ret = __GT;
			break;
		case 0xFC:
			ret = __LT;
			break;
		case 0xFD:
			ret = __AND;
			break;
		case 0xFE:
			ret = __OR;
			break;
		case 0xFF:
			ret = __NEV;
			break;
		case 0x03:
			ret = __ECC256;
			break;
		case 0x04:
			ret = __ECC384;
			break;
		case 0xE2:
			ret = __SHA256;
			break;
	}
	return ret;
}

static char* __decodeAC_E1(uint8_t data)
{
	char *ret;
	uint16_t i=0; 
	
	if (data & 0x01) // Auth = 0x01 
	{
		strcpy((__E1+i),__AUTH);
		i += (sizeof(__AUTH));
	}
	if (data & 0x02) // Enc = 0x02 
	{
		__E1[i-1] = '/';
		strcpy((__E1+i),__ENC);
		i += sizeof(__ENC);
	}
	if (data & 0x04) // HFU = 0x04 
	{
		__E1[i-1] = '/';
		strcpy((__E1+i),__HFU);
		i += sizeof(__HFU);
	}
	if (data & 0x08) // DM = 0x08 
	{
		__E1[i-1] = '/';
		strcpy((__E1+i),__DM);
		i += sizeof(__DM);
	}
	if (data & 0x10) // SIGN = 0x10 
	{
		__E1[i-1] = '/';
		strcpy((__E1+i),__SIGN);
		i += sizeof(__SIGN);
	}
	if (data & 0x20) // AGREE = 0x20 
	{
		__E1[i-1] = '/';
		strcpy((__E1+i),__AGREE);
		i += sizeof(__AGREE);
	}
	ret = __E1;
	return ret;
}

void trustXdecodeMetaData(uint8_t * metaData)
{
	uint16_t i,j;
	uint16_t metaLen;
	uint16_t len;
	uint8_t LcsO;
	uint16_t maxDataObjSize;
	
	if(*metaData == 0x20)
	{
		i = 1;
		metaLen = *(metaData+(i++));
		while(i < metaLen)
		{
			
			switch(*(metaData+(i++)))
			{
				case 0xC0:
					// len is always 1
					len = *(metaData+(i++));
					LcsO = *(metaData+(i++));
					printf("LcsO:0x%.2X, ",LcsO);
					break;
				
				case 0xC4:
					// len is 1 or 2
					len = *(metaData+(i++));
					maxDataObjSize = *(metaData+(i++));
					if (len == 2)
						maxDataObjSize = (maxDataObjSize<<8) + *(metaData+(i++));
					printf("Max:%d, ",maxDataObjSize);
					break;
				
				case 0xC5:
					len = *(metaData+(i++));
					maxDataObjSize = *(metaData+(i++));
					if (len == 2)
						maxDataObjSize = (maxDataObjSize<<8) + *(metaData+(i++));
					printf("Used:%d, ",maxDataObjSize);
					break;
				
				case 0xD0:
					len = *(metaData+(i++));
					printf("C:");
					for (j=0; j<len;j++)
					{
						if((*(metaData+(i)) == 0x00)||(*(metaData+(i)) == 0xff) )
							printf("%s, ",__decodeAC(*(metaData+(i++))));
						else
						{
							if((*(metaData+(i)) > 0xfa) && 
								(*(metaData+(i)) <= 0xff))
							{
								printf("%s",__decodeAC(*(metaData+(i++))));
							}
							else
							{
								if((*(metaData+(i)) == 0x70)||(*(metaData+(i)) == 0xe1))
									printf("%s",__decodeAC(*(metaData+(i++))));
								else
								{
									printf("%d",*(metaData+(i++)));
									if ((len-j) < 3)
										printf(", ");
								}
							}
						}
					}
					break;
				
				case 0xD1:
					len = *(metaData+(i++));
					printf("R:");
					for (j=0; j<len;j++)
					{
						if((*(metaData+(i)) == 0x00)||(*(metaData+(i)) == 0xff) )
							printf("%s",__decodeAC(*(metaData+(i++))));
						else
						{
							if((*(metaData+(i)) > 0xfa) && 
								(*(metaData+(i)) <= 0xff))
							{
								printf("%s",__decodeAC(*(metaData+(i++))));
							}
							else
							{
								if((*(metaData+(i)) == 0x70)||(*(metaData+(i)) == 0xe1))
									printf("%s",__decodeAC(*(metaData+(i++))));
								else
								{
									printf("%d",*(metaData+(i++)));
									if ((len-j) < 3)
										printf(", ");
								}
							}
						}
					}
					break;
				
				case 0xD2:
					len = *(metaData+(i++));
					printf("D:");
					for (j=0; j<len;j++)
					{
						if((*(metaData+(i)) == 0x00)||(*(metaData+(i)) == 0xff) )
							printf("%s",__decodeAC(*(metaData+(i++))));
						else
						{
							if((*(metaData+(i)) > 0xfa) && 
								(*(metaData+(i)) <= 0xff))
							{
								printf("%s",__decodeAC(*(metaData+(i++))));
							}
							else
							{
								if((*(metaData+(i)) == 0x70)||(*(metaData+(i)) == 0xe1))
									printf("%s",__decodeAC(*(metaData+(i++))));
								else
								{
									printf("%d",*(metaData+(i++)));
									if ((len-j) < 3)
										printf(", ");
								}
							}
						}
					}				
					break;
					
				case 0xE0:
					// len is always 1
					len = *(metaData+(i++));
					printf("Algo:%s, ",__decodeAC(*(metaData+(i++))));				
					break;
					
				case 0xE1:
					// len is always 1
					len = *(metaData+(i++));
					printf("Key:%s, ",__decodeAC_E1(*(metaData+(i++))));				
					break;
				
				default:
					i = metaLen;
					break;
					
			}
		}
	printf("\n");	
	}
}

/**********************************************************************
* trustX_debugprintHex()
**********************************************************************/
void trustXHexDump(uint8_t *pdata, uint32_t len)
{
	uint32_t i, j;

	printf("\t");	
	j=0;
	for (i=0; i < len; i++)
	{
		printf("%.2x ",*(pdata+i));
		if (j < 15)	
		{
			j++;
		}
		else
		{
			j=0;
			printf("\n\t");
		}
	}
	printf("\n");
}

uint16_t trustXWritePEM(uint8_t *buf, uint32_t len, const char *filename, char *name)
{
	FILE *fp;
	char header[] = "";

	fp = fopen(filename,"wb");
	if (!fp)
	{
		TRUSTX_HELPER_ERRFN("error creating file!!\n");
		return 1;
	}

	//Write cert to file
	PEM_write(fp, name,header,buf,(long int)len);				

	fclose(fp);

	return 0;
}

uint16_t trustXWriteDER(uint8_t *buf, uint32_t len, const char *filename)
{
	FILE *fp;

	fp = fopen(filename,"wb");
	if (!fp)
	{
		TRUSTX_HELPER_ERRFN("error creating file!!\n");
		return 1;
	}

	//Write cert to file
	fwrite(buf, 1, len, fp);				

	fclose(fp);

	return 0;
}

uint16_t trustXReadPEM(uint8_t *buf, uint32_t *len, const char *filename, char *name)
{
	FILE *fp;
	char *tempName;
	char *header;
	uint8_t *data;
	long int dataLen;

	fp = fopen(filename,"r");
	if (!fp)
	{
		printf("failed to open file %s\n",filename);
		return 1;
	}
	
	dataLen = 0;
	PEM_read(fp, &tempName,&header,&data,&dataLen);
	memcpy(buf,data,dataLen);
	*len = dataLen;
	
	strcpy(name,tempName);
	
	fclose(fp);
	return 0;
}

uint16_t trustXReadDER(uint8_t *buf, uint32_t *len, const char *filename)
{
	
	FILE *fp;
	uint16_t tempLen;
	uint8_t data[2048];

	//open 
	fp = fopen((const char *)filename,"rb");
	if (!fp)
	{
		return 1;
	}

	//Read file
	tempLen = fread(data, 1, sizeof(data), fp); 
	if (tempLen > 0)
	{
		memcpy(buf,data,tempLen);
		*len = tempLen;
	}
	else
	{
		*len = 0;
	}

	fclose(fp);

	return 0;
}

uint16_t trustXWriteX509PEM(X509 *x509, const char *filename)
{
	FILE *x509file;
	uint16_t ret;

	//create x509 file pem
	x509file = fopen(filename,"wb");
	if (!x509file)
	{
		printf("error creating x509 file!!\n");
		return 1;
	}

	//Write cert to file
	ret = PEM_write_X509(x509file, x509);
	fclose(x509file);
	if (!ret)
	{
		printf("Unable Cert to write to file!!\n");
		return 1;
	}

	return 0;

}

uint16_t trustXReadX509PEM(X509 **x509, const char *filename)
{
	FILE *x509file;

	//open x509 file pem
	x509file = fopen(filename,"rb");
	if (!x509file)
	{
		printf("error reading x509 file!!\n");
		return 1;
	}

	//Read file to cert 
	*x509 = PEM_read_X509(x509file, NULL, 0, NULL);
	fclose(x509file);
	if (x509 == NULL)
	{
		printf("Unable read cert from file!!\n");
		return 1;
	}

	return 0;

}

/**********************************************************************
* trustX_readCert()
**********************************************************************/
optiga_lib_status_t trustX_readCert(uint16_t oid, uint8_t* p_cert, uint32_t* length)
{
    uint16_t offset, bytes_to_read;
    uint8_t read_data_buffer[5000];
    uint16_t i;
    
    optiga_lib_status_t return_status;

    do
    {
        offset = 0x00;
        bytes_to_read = sizeof(read_data_buffer);

        return_status = optiga_util_read_data(oid,
                                              offset,
                                              read_data_buffer,
                                              &bytes_to_read);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
			//Reading the data object failed.
			TRUSTX_HELPER_ERRFN("optiga_util_read_data : FAIL!!!\n");
            break;
        }
        
	for (i=0;i<bytes_to_read;i++)
	{
		*(p_cert+i) = read_data_buffer[i];
	}

    } while(FALSE);

    return return_status;

}

/**********************************************************************
* trustX_readUID()
**********************************************************************/
optiga_lib_status_t trustX_readUID(utrustX_UID_t *UID)
{
    uint16_t offset, bytes_to_read;
    uint16_t optiga_oid;
    uint8_t read_data_buffer[1024];

    optiga_lib_status_t return_status;

    uint16_t i;

    do
    {
        //Read device UID
        optiga_oid = eCOPROCESSOR_UID;
        offset = 0x00;
        bytes_to_read = sizeof(read_data_buffer);

        return_status = optiga_util_read_data(optiga_oid,
                                              offset,
                                              read_data_buffer,
                                              &bytes_to_read);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
			//Reading the data object failed.
			TRUSTX_HELPER_ERRFN("optiga_util_read_data : FAIL!!!\n");
            break;
        }

	for (i=0;i<bytes_to_read;i++)
	{
		UID->b[i] = read_data_buffer[i];
	}

    } while(FALSE);

    return return_status;
}

/**********************************************************************
* trustX_Open()
**********************************************************************/
optiga_lib_status_t trustX_Open(void)
{
	int32_t status = (int32_t) OPTIGA_LIB_ERROR;
		
	TRUSTX_HELPER_DBGFN(">> Enter trustX_Open()\n");
	do
	{
		i2c_if = dev;
		pal_gpio_init();
		pal_os_event_init();

		if (pal_init() != PAL_STATUS_SUCCESS)
		{
			TRUSTX_HELPER_ERRFN( "Failure: pal_init()!!!\n\r");
			break;
		}
			
		status = optiga_util_open_application(&optiga_comms);
		
		if(OPTIGA_LIB_SUCCESS != status)
		{
			TRUSTX_HELPER_ERRFN( "Failure: optiga_util_open_application(): 0x%04X\n\r", status);
			break;
		}
		
		TRUSTX_HELPER_DBGFN("optiga_util_open_application(): passed.\n");
		status = OPTIGA_LIB_SUCCESS;
	} while(0);

	TRUSTX_HELPER_DBGFN("<< Exit trustX_Open()\n");

	return status;	
}

/**********************************************************************
* trustX_Close()
**********************************************************************/
void trustX_Close(void)
{
	int32_t status = (int32_t) OPTIGA_LIB_ERROR;
	
	TRUSTX_HELPER_DBGFN(">");	
	do
	{

		status = optiga_comms_close(&optiga_comms);
		if(OPTIGA_LIB_SUCCESS != status)
		{
			TRUSTX_HELPER_ERRFN( "Failure: optiga_comms_close(): 0x%04X\n\r", status);
			break;
		}
		status = OPTIGA_LIB_SUCCESS;
		pal_os_event_stop();
		pal_gpio_deinit();

	} while(0);

	TRUSTX_HELPER_DBGFN("TrustX Closed.\n");
	TRUSTX_HELPER_DBGFN("<");	
	//return status;
}
