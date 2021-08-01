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
#include <string.h>

#include "optiga/ifx_i2c/ifx_i2c_config.h"
#include "optiga/optiga_util.h"

#include "trustx.h"

typedef struct _OPTFLAG {
	uint16_t	i2cbus		: 1;
	uint16_t	dummy1		: 1;
	uint16_t	dummy2		: 1;
	uint16_t	dummy3		: 1;
	uint16_t	dummy4		: 1;
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

void helpmenu(void)
{
	printf("\nHelp menu: trustx_readmetadata_cert <option> ...<option>\n");
	printf("option:- \n");
	printf("-b Set I2C bus (Default %s) \n", pTrustX_I2C_Bus);
	printf("-h Print this help menu\n");
}

int main (int argc, char **argv)
{
	int option = 0;
	uOptFlag.all = 0;
	optiga_lib_status_t return_status;
	uint16_t i, j, k, skip_flag;
	
	uint16_t bytes_to_read;
	uint16_t optiga_oid;
	uint8_t read_data_buffer[1024];

	do // Begin of DO WHILE(FALSE) for error handling.
	{
		// ---------- Check for command line parameters ----------
		if (1 == argc)
		{
		}

		// ---------- Command line parsing with getopt ----------
		opterr = 0; // Disable getopt error messages in case of unknown parameters

		// Loop through parameters with getopt.
		while (-1 != (option = getopt(argc, argv, "b:h")))
		{
			switch (option)
			{
				case 'b': // Set I2C Bus
					uOptFlag.flags.i2cbus = 1;
					strcpy(pTrustX_I2C_Bus,optarg);
					break;
				case 'h': // Print Help Menu
					helpmenu();
					exit(0);
				default:  // Any other command Print Help Menu
					helpmenu();
					exit(0);
					break;
			}
		}
	} while (0); // End of DO WHILE FALSE loop.

	return_status = trustX_Open();
	if (return_status != OPTIGA_LIB_SUCCESS)
		exit(1);

	do
	{
		printf("===========================================\n");	
		


		for (i = 0; i < (0xE0EF-0xE0E0+1); i++) // Limit to Obj
		{
			optiga_oid = 0xE0E0;
			skip_flag = 0;	
			optiga_oid += i;
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
					//i = 0x1BF;
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
					skip_flag = 2;
			}

			if(skip_flag == 0 || skip_flag == 1)
			{
				bytes_to_read = sizeof(read_data_buffer);
				return_status = optiga_util_read_metadata(optiga_oid,
															read_data_buffer,
															&bytes_to_read);

				if (return_status != OPTIGA_LIB_SUCCESS)
				{
					printf("Error!!! [0x%.8X]\n", return_status);
				}
				else
				{
					k=0;
					printf("[Size %.4d] : \n\t", bytes_to_read);
					
					for (j=0;j<bytes_to_read;j++)
					{
						printf("%.2X ", read_data_buffer[j]);
						if(k < 25)
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
		}
	}while(FALSE);

	printf("===========================================\n");	

	trustX_Close();
	return 0;
}
