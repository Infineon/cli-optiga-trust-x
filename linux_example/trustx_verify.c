/**
* MIT License
*
* Copyright (c) 2018 Infineon Technologies AG
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
#include <stdint.h>

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

#include "optiga/ifx_i2c/ifx_i2c_config.h"
#include "optiga/optiga_util.h"

#include "trustx.h"

typedef struct _OPTFLAG {
	uint16_t	verify		: 1;
	uint16_t	input		: 1;
	uint16_t	signature	: 1;
	uint16_t	hash		: 1;
	uint16_t	pubkey		: 1;
	uint16_t	dummy5		: 1;
	uint16_t	dummy6		: 1;
	uint16_t	dummy7		: 1;
	uint16_t	dummy8		: 1;
	uint16_t	dummy9		: 1;
	uint16_t	dummy10		: 1;
	uint16_t	dummy11		: 1;
	uint16_t	dummy12		: 1;
	uint16_t	dummy13		: 1;
	uint16_t	dummy14		: 1;
	uint16_t	dummy15		: 1;
}OPTFLAG;

union _uOptFlag {
	OPTFLAG	flags;
	uint16_t	all;
} uOptFlag;

void _helpmenu(void)
{
	printf("\nHelp menu: trustx_verify <option> ...<option>\n");
	printf("option:- \n");
	printf("-k <OID Key>   : Read Certificate from OID 0xNNNN \n");
	printf("-i <filename>  : Input Data file\n");
	printf("-s <signature> : Signature file\n");
	printf("-p <pubkey>    : Host Pubkey\n");
	printf("-H             : Hash before sign\n");
	printf("-h             : Print this help \n");
}

static uint32_t _ParseHexorDec(const char *aArg)
{
	uint32_t value;

	if ((strncmp(aArg, "0x",2) == 0) ||(strncmp(aArg, "0X",2) == 0))
		sscanf(aArg,"%x",&value);
	else
		sscanf(aArg,"%d",&value);

	return value;
}

void _hexdump(uint8_t *data, uint16_t len)
{
	uint16_t j,k;

	printf("\t");
	k=0;
	for (j=0;j<len;j++)
	{
		printf("%.2X ", data[j]);
		if(k < 15)
		{
			k++;
		}	
		else
		{
			printf("\n\t");
			k=0;
		}
	}
	printf("\n");
}

uint16_t _writeTo(uint8_t *buf, uint32_t len, const char *filename)
{
	FILE *datafile;

	//create 
	datafile = fopen(filename,"wb");
	if (!datafile)
	{
		return 1;
	}

	//Write to file
	fwrite(buf, 1, len, datafile);
	fclose(datafile);

	return 0;

}

static uint16_t _readFrom(uint8_t *data, uint8_t *filename)
{
	
	FILE *datafile;
	uint16_t len;
	uint8_t buf[2048];
	uint16_t ret;

	//open 
	datafile = fopen((const char *)filename,"rb");
	if (!datafile)
	{
		return 1;
	}

	//Read file
	len = fread(buf, 1, sizeof(buf), datafile); 
	if (len > 0)
	{
		ret = len;
		memcpy(data,buf,len);
	}

	fclose(datafile);

	return ret;

}



int main (int argc, char **argv)
{
	optiga_lib_status_t return_status;

	uint16_t optiga_oid;
	uint8_t signature [100];     //To store the signture generated
    uint16_t signatureLen = sizeof(signature);
    uint8_t digest[100];
    uint16_t digestLen;
    uint8_t pubkey[2048];
    uint32_t pubkeyLen;

    char *inFile = NULL;
    char *signatureFile = NULL;
    char *pubkeyFile = NULL;
    char name[100];
    
	int option = 0;                    // Command line option.


/***************************************************************
 * Getting Input from CLI
 **************************************************************/
	uOptFlag.all = 0;
    printf("\n");
    do // Begin of DO WHILE(FALSE) for error handling.
    {
		// ---------- Check for command line parameters ----------
		
        if (argc < 2)
        {
			_helpmenu();
			exit(0);
		}

        // ---------- Command line parsing with getopt ----------
        opterr = 0; // Disable getopt error messages in case of unknown parameters

        // Loop through parameters with getopt.
        while (-1 != (option = getopt(argc, argv, "k:i:s:p:Hh")))
        {
			switch (option)
            {
				case 'k': // Cert OID 
					uOptFlag.flags.verify = 1;
					optiga_oid = _ParseHexorDec(optarg);			 	
					break;
				case 'i': // Input
					uOptFlag.flags.input = 1;
					inFile = optarg;			 	
					break;
				case 's': // Signature
					uOptFlag.flags.signature = 1;
					signatureFile = optarg;			 	
					break;
				case 'p': // Host Pubkey
					uOptFlag.flags.pubkey = 1;
					pubkeyFile = optarg;			 	
					break;
				case 'H': // Input
					uOptFlag.flags.hash = 1;		 	
					break;
				case 'h': // Print Help Menu
				default:  // Any other command Print Help Menu
					_helpmenu();
					exit(0);
					break;
			}
		}
    } while (0); // End of DO WHILE FALSE loop.
 

/***************************************************************
 * Example 
 **************************************************************/
	return_status = trustX_Open();
	if (return_status != OPTIGA_LIB_SUCCESS)
		exit(1);
	printf("========================================================\n");	

	do
	{
		if(uOptFlag.flags.verify == 1)
		{
			if(uOptFlag.flags.input != 1)
			{
				printf("Input filename missing!!!\n");
				break;
			}

			if(uOptFlag.flags.signature != 1)
			{
				printf("Signature filename missing!!!\n");
				break;
			}

			printf("OID Cert 0x%.4X\n",optiga_oid);
			printf("Input File Name : %s \n", inFile);
			printf("Signature File Name : %s \n", signatureFile);

			digestLen = _readFrom(digest, (uint8_t *) inFile);
			if (digestLen == 0)
			{
				printf("Error reading input file!!!\n");
				break;				
			}

			signatureLen = _readFrom(signature, (uint8_t *) signatureFile);
			if (signatureLen == 0)
			{
				printf("Error signature reading file!!!\n");
				break;				
			}

			
			printf("Input data : \n");
			_hexdump(digest,digestLen);
			
			printf("Signature : \n");
			_hexdump(signature,signatureLen);					

			return_status = optiga_crypt_ecdsa_verify (digest,
														digestLen,
														signature,
														signatureLen,
														OPTIGA_CRYPT_OID_DATA,
														&optiga_oid);

			if (return_status != OPTIGA_LIB_SUCCESS)
			{
				printf("Error!!! [0x%.8X]\n",return_status);
			}
			else
			{
				printf("Verify Success.\n");
			}
		}

		if(uOptFlag.flags.pubkey == 1)
		{
			if(uOptFlag.flags.input != 1)
			{
				printf("Input filename missing!!!\n");
				break;
			}

			if(uOptFlag.flags.signature != 1)
			{
				printf("Signature filename missing!!!\n");
				break;
			}

			printf("Pubkey file : %s\n",pubkeyFile);
			printf("Input File Name : %s \n", inFile);
			printf("Signature File Name : %s \n", signatureFile);

			digestLen = _readFrom(digest, (uint8_t *) inFile);
			if (digestLen == 0)
			{
				printf("Error reading input file!!!\n");
				break;				
			}

			signatureLen = _readFrom(signature, (uint8_t *) signatureFile);
			if (signatureLen == 0)
			{
				printf("Error signature reading file!!!\n");
				break;				
			}

            trustXReadPEM(pubkey, &pubkeyLen, pubkeyFile, name);
            if (strcmp(name, "PUBLIC KEY"))
            {
				printf("Not Public File!!!\n");
				break;
			}
			if (pubkeyLen == 0)
			{
				printf("Error pubkey reading file!!!\n");
				break;				
			}
			
			printf("Input data : \n");
			_hexdump(digest,digestLen);	

			printf("Signature : \n");
			_hexdump(signature,signatureLen);	

			printf("Pub key : \n");
			_hexdump((pubkey+23),pubkeyLen-23);	

    public_key_from_host_t public_key_details = {
                                                 pubkey+23,
                                                 pubkeyLen-23,
												OPTIGA_ECC_NIST_P_256
                                                };

			return_status = optiga_crypt_ecdsa_verify (digest,
														digestLen,
														signature,
														signatureLen,
														OPTIGA_CRYPT_HOST_DATA,
														 &public_key_details);

			if (return_status != OPTIGA_LIB_SUCCESS)
			{
				printf("Error!!! [0x%.8X]\n",return_status);
			}
			else
			{
				printf("Verify Success.\n");
			}
		}


	}while(0);
	printf("========================================================\n");	
	
	trustX_Close();

	return 0;
}
