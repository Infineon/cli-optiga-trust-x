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
	printf("\nHelp menu: trustx_chipinfo <option> ...<option>\n");
	printf("option:- \n");
	printf("-b Set I2C bus (Default %s) \n", pTrustX_I2C_Bus);
	printf("-h Print this help menu\n");
}

int main (int argc, char **argv)
{
	int option = 0;
	optiga_lib_status_t return_status;
	utrustX_UID_t UID;
	uOptFlag.all = 0;

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

	return_status = trustX_readUID(&UID);
        if (return_status != OPTIGA_LIB_SUCCESS)
        {
            printf("readUID [0xE0C2]: FAIL!!!\n");
        }
	else
	{
	    printf("Read Chip Info [0xE0C2]: Success.\n");
	    printf("===========================================\n");
        printf("CIM Identifier             [bCimIdentifer]: 0x%.2x\n", UID.st.bCimIdentifer);
	    printf("Platform Identifer   [bPlatformIdentifier]: 0x%.2x\n", UID.st.bPlatformIdentifier);
	    printf("Model Identifer         [bModelIdentifier]: 0x%.2x\n", UID.st.bModelIdentifier);
	    printf("ID of ROM mask                  [wROMCode]: 0x%.2x%.2x\n", 
								UID.st.wROMCode[0],
								UID.st.wROMCode[1]);
	    printf("Chip Type                    [rgbChipType]: 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
								UID.st.rgbChipType[0],
								UID.st.rgbChipType[1],
								UID.st.rgbChipType[2],
								UID.st.rgbChipType[3],
								UID.st.rgbChipType[4],
								UID.st.rgbChipType[5]);
	    printf("Batch Number              [rgbBatchNumber]: 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
								UID.st.rgbBatchNumber[0],
								UID.st.rgbBatchNumber[1],
								UID.st.rgbBatchNumber[2],
								UID.st.rgbBatchNumber[3],
								UID.st.rgbBatchNumber[4],
								UID.st.rgbBatchNumber[5]);
	    printf("X-coordinate              [wChipPositionX]: 0x%.2x%.2x\n", 
								UID.st.wChipPositionX[0],
								UID.st.wChipPositionX[1]);
	    printf("Y-coordinate              [wChipPositionY]: 0x%.2x%.2x\n",
								UID.st.wChipPositionY[0],
								UID.st.wChipPositionY[1]);
	    printf("Firmware Identifier [dwFirmwareIdentifier]: 0x%.2x%.2x%.2x%.2x\n", 
								UID.st.dwFirmwareIdentifier[0],
								UID.st.dwFirmwareIdentifier[1],
								UID.st.dwFirmwareIdentifier[2],
								UID.st.dwFirmwareIdentifier[3]);
	    printf("Build Number                 [rgbESWBuild]: %.2x %.2x\n",
								UID.st.rgbESWBuild[0],
								UID.st.rgbESWBuild[1]);
	}
	printf("\n");

	printf("Chip software build: \n");  
	if ((UID.st.rgbESWBuild[0] == 0x05) && (UID.st.rgbESWBuild[1] == 0x10))
		printf("OPTIGA(TM) Trust X; Firmware Version: 1.0.510\n");
	else if ((UID.st.rgbESWBuild[0] == 0x07) && (UID.st.rgbESWBuild[1] == 0x15))
		printf("OPTIGA(TM) Trust X; Firmware Version: 1.1.715\n");
	else if ((UID.st.rgbESWBuild[0] == 0x10) && (UID.st.rgbESWBuild[1] == 0x48))
		printf("OPTIGA(TM) Trust X; Firmware Version: 1.2.1048\n");
	else if ((UID.st.rgbESWBuild[0] == 0x11) && (UID.st.rgbESWBuild[1] == 0x12))
		printf("OPTIGA(TM) Trust X; Firmware Version: 1.30.1112\n");
	else if ((UID.st.rgbESWBuild[0] == 0x11) && (UID.st.rgbESWBuild[1] == 0x18))
		printf("OPTIGA(TM) Trust X; Firmware Version: 1.40.1118\n");
	else if ((UID.st.rgbESWBuild[0] == 0x08) && (UID.st.rgbESWBuild[1] == 0x09))
		printf("OPTIGA(TM) Trust M rev.1; Firmware Version: 1.30.809 (Attention: not all features are supported)\n");
	else
		printf("Unknown\n");
	printf("===========================================\n");	


	trustX_Close();
	return 0;
}
