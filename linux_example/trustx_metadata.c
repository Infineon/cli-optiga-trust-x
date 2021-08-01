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
#include <stdint.h>
#include <string.h>

#include "optiga/ifx_i2c/ifx_i2c_config.h"
#include "optiga/optiga_util.h"

#include "trustx.h"

static const uint8_t __bALW[] = {0x01,0x00}; // alway read or write
static const uint8_t __bNEV[] = {0x01,0xff}; // disable read or write
static const uint8_t __bON_TERMINATE[] = {0x03,0xE1,0xFC,0x09}; // disable on terminate
static const uint8_t __bLOCK[] = {0x01,0x07}; // lock LcsO
static const uint8_t __bTERMINATE[] = {0x01,0x0F}; // terminate LcsO

typedef struct _OPTFLAG {
	uint16_t	i2cbus		: 1;
	uint16_t	read		: 1;
	uint16_t	write		: 1;
	uint16_t	lcschange	: 1;
	uint16_t	lcsread		: 1;
	uint16_t	lcslock		: 1;
	uint16_t	lcsterminate: 1;
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

static void _helpmenu(void)
{
	printf("\nHelp menu: trustx_metadata <option> ...<option>\n");
	printf("option:- \n");
	printf("-b Set I2C bus (Default %s) \n", pTrustX_I2C_Bus);
	printf("-r <OID>  : Read metadata of OID 0xNNNN \n");
	printf("-w <OID>  : Write metadata of OID\n");
	printf("-C <data> : Set Change mode (a:allow change,\n"); 
	printf("                             n:disable change,\n"); 
	printf("                             t:disable change on termination,\n");
	printf("                             f:<input file for complex setting>)\n");
	printf("-R <data> : Set Read mode (a:allow read,\n"); 
	printf("                           t:disable read on termination\n");
	printf("                           f:<input file for complex setting>)\n");
	printf("-L        : Lock OID metadata \n");
	printf("-T        : TERMINATE OID \n");	
	printf("-h        : Print this help \n");
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
		printf("File open error!!!\n");
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
	uint16_t bytes_to_read;
    uint16_t optiga_oid;
    uint8_t read_data_buffer[2048];
    uint8_t mode[200];
    uint16_t modeLen;
    uint16_t j,k;
    uint8_t *lcsChange = NULL;
    uint8_t *lcsRead = NULL;
    uint8_t tempData[20];
     
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
        while (-1 != (option = getopt(argc, argv, "r:w:C:R:LTh")))
        {
			switch (option)
			{
				case 'b': // Set I2C Bus
					uOptFlag.flags.i2cbus = 1;
					strcpy(pTrustX_I2C_Bus,optarg);
					break;
				case 'r': // Read
					uOptFlag.flags.read = 1;
					optiga_oid = _ParseHexorDec(optarg);			 	
					break;
				case 'w': // Write
					uOptFlag.flags.write = 1;	
					optiga_oid = _ParseHexorDec(optarg);								 	
					break;
				case 'C': // Change setting
					uOptFlag.flags.lcschange = 1;
					lcsChange = (uint8_t *)optarg;
					if((lcsChange[0] != 'a')&&(lcsChange[0] != 'n')&&
						(lcsChange[0] != 't')&&(lcsChange[0] != 'f'))
					{
						_helpmenu();
						exit(0);						
					}				
					break;
				case 'R': // Read setting
					uOptFlag.flags.lcsread = 1;
					lcsRead = (uint8_t *)optarg;
					if((lcsRead[0] != 'a')&&(lcsRead[0] != 't')&&
						(lcsRead[0] != 'f'))
					{
						_helpmenu();
						exit(0);						
					}
					break;					
				case 'L': // Lock
					uOptFlag.flags.lcslock = 1;
					break;					
				case 'T': // Terminate
					uOptFlag.flags.lcsterminate = 1;
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

	switch (optiga_oid)
	{
		case 0xE0C0:
			printf("Global Life Cycle Status    [0x%.4X] ", optiga_oid);
			break;
		case 0xE0C1:
			printf("Global Security Status      [0x%.4X] ", optiga_oid);
			break;
		case 0xE0C2:
			printf("UID                         [0x%.4X] ", optiga_oid);
			break;
		case 0xE0C3:
			printf("Sleep Mode Activation Delay [0x%.4X] ", optiga_oid);
			break;
		case 0xE0C4:
			printf("Current Limitation          [0x%.4X] ", optiga_oid);
			break;
		case 0xE0C5:
			printf("Security Event Counter      [0x%.4X] ", optiga_oid);
			break;
		case 0xE0C6:
			printf("Max Com Buffer Size         [0x%.4X] ", optiga_oid);
			break;
		case 0xE0E0:
			printf("Device Public Key IFX       [0x%.4X] ", optiga_oid);
			break;
		case 0xE0E1:
		case 0xE0E2:
		case 0xE0E3:
			printf("Device Public Key           [0x%.4X] ", optiga_oid);
			break;
		case 0xE0E8:
			printf("Root CA Public Key Cert1    [0x%.4X] ", optiga_oid);
			break;
		case 0xE0EF:
			printf("Root CA Public Key Cert8    [0x%.4X] ", optiga_oid);
			break;
		case 0xE0F0:
			printf("Device Privte Key 1         [0x%.4X] ", optiga_oid);
			break;
		case 0xE0F1:
		case 0xE0F2:
		case 0xE0F3:
			printf("Device Privte Key x         [0x%.4X] ", optiga_oid);
			break;
		case 0xE100:
		case 0xE101:
		case 0xE102:
		case 0xE103:
			printf("Session Context x           [0x%.4X] ", optiga_oid);
			break;					
		case 0xF1C0:
			printf("Application Life Cycle Sts  [0x%.4X] ", optiga_oid);
			break;					
		case 0xF1C1:
			printf("Application Security Sts    [0x%.4X] ", optiga_oid);
			break;					
		case 0xF1C2:
			printf("Application Error Codes     [0x%.4X] ", optiga_oid);
			break;					
		case 0xF1D0:
		case 0xF1D1:
		case 0xF1D2:
		case 0xF1D3:
		case 0xF1D4:
		case 0xF1D5:
		case 0xF1D6:
		case 0xF1D7:
		case 0xF1D8:
		case 0xF1D9:
		case 0xF1DA:
		case 0xF1DB:
		case 0xF1DC:
		case 0xF1DD:
		case 0xF1DE:
		case 0xF1DF:
			printf("App DataStrucObj type 1     [0x%.4X] ", optiga_oid);
			break;					
		case 0xF1E0:
		case 0xF1E1:
			printf("App DataStrucObj type 2     [0x%.4X] ", optiga_oid);
			break;						
		default:
			break;
	}

	do
	{
		if(uOptFlag.flags.read == 1)
		{
		
			bytes_to_read = sizeof(read_data_buffer);
			return_status = optiga_util_read_metadata(optiga_oid,
														read_data_buffer,
														&bytes_to_read);

			if (return_status != OPTIGA_LIB_SUCCESS)
			{
				printf("Error!!! [0x%.8X]\n",return_status);
			}
			else
			{
				k=0;
				printf("[Size %.4d] : ", bytes_to_read);
				
				printf("\n\t");
					
				for (j=0;j<bytes_to_read;j++)
				{
					printf("%.2X ", read_data_buffer[j]);
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
				printf("\n\t");
				trustXdecodeMetaData(read_data_buffer);
				printf("\n");
			}	
		}

		if(uOptFlag.flags.write == 1)
		{
			if (!(uOptFlag.all & 0x003C))
			{
				printf ("\nMust at least input -L,-T,-C or -R!!! \n");
				exit(1);
			}
			
			modeLen=0;
			mode[modeLen++] = 0x20;
			modeLen++; // skip the len input first
			if ((uOptFlag.flags.lcslock == 1) || (uOptFlag.flags.lcsterminate == 1))
			{
				mode[modeLen++] = 0xC0; // LcsO
				if(uOptFlag.flags.lcsterminate == 1)
				{
					memcpy((mode+modeLen),__bTERMINATE,sizeof(__bTERMINATE));
					modeLen += sizeof(__bTERMINATE);
				}
				else
				{
					memcpy((mode+modeLen),__bLOCK,sizeof(__bLOCK));
					modeLen += sizeof(__bLOCK); 
				}	 
			}
			
			if (uOptFlag.flags.lcschange == 1)
			{
				mode[modeLen++] = 0xD0; // Change Access Condition
				switch(lcsChange[0])
				{
					case 'a':
						memcpy((mode+modeLen),__bALW,sizeof(__bALW));
						modeLen += sizeof(__bALW);
						break;
					case 'n':
						memcpy((mode+modeLen),__bNEV,sizeof(__bNEV));
						modeLen += sizeof(__bNEV); 
						break;
					case 't':
						memcpy((mode+modeLen),__bON_TERMINATE,sizeof(__bON_TERMINATE));
						modeLen += sizeof(__bON_TERMINATE); 
						break;
					case 'f':
						if (lcsChange[1] == ':')
						{
							_readFrom(tempData, (lcsChange+2));
							if((tempData[0]+1) < 0x0b)
							{
								memcpy((mode+modeLen),tempData,(tempData[0]+1));
								modeLen += (tempData[0]+1);							
							}
							else
							{
								printf ("\nInvalid input!!! \n");
								exit(1);														
							}
						}
						else
						{
							printf ("\nInvalid f parameter input!!! \n");
							exit(1);							
						}
						break;
				}
			}

			if (uOptFlag.flags.lcsread == 1)
			{
				mode[modeLen++] = 0xD1; // Read Access Condition
				switch(lcsRead[0])
				{
					case 'a':
						memcpy((mode+modeLen),__bALW,sizeof(__bALW));
						modeLen += sizeof(__bALW); 
						break;
					case 't':
						memcpy((mode+modeLen),__bON_TERMINATE,sizeof(__bON_TERMINATE));
						modeLen += sizeof(__bON_TERMINATE); 
						break;
					case 'f':
						if (lcsRead[1] == ':')
						{
							_readFrom(tempData, (lcsRead+2));
							if((tempData[0]+1) < 0x0b)
							{
								memcpy((mode+modeLen),tempData,(tempData[0]+1));
								modeLen += (tempData[0]+1);							
							}
							else
							{
								printf ("\nInvalid input!!! \n");
								exit(1);														
							}
						}
						else
						{
							printf ("\nInvalid f parameter input!!! \n");
							exit(1);							
						}
						break;
				}
				printf("testpoint 1 \n");
			}
			mode[1] = modeLen-2;
			printf("\n");
			_hexdump(mode,modeLen);
			printf("\t");
			trustXdecodeMetaData(mode);

			return_status = optiga_util_write_metadata(optiga_oid,
													   mode,
													   modeLen);

			if (return_status != OPTIGA_LIB_SUCCESS)
			{
				printf("Error!!! [0x%.8X]\n",return_status);
			}
			else
			{
				printf("Write Success.\n");
			}				
		}
	} while(0);
	printf("========================================================\n");	
	
	trustX_Close();
	return 0;
}
