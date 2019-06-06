# Command Line Interface (CLI) & OpenSSL Engine for OPTIGA™ Trust X security solution

------

[About](#about)

​		[Prerequisites](#prerequisites)

​		[Contents of Package](#contents-of-package)

[Getting Started](#getting-started)

​		[Before Building the tools](#before-building-the-tools)

​				[Patching the trustx_lib](#patching-the-trustx_lib)

​		[First time building the library](#first-time-building-the-library)

​		[Building the engine](#building-the-engine)

​		[Build the command line tools](#build-the-command-line-tools)

[CLI Tools Usage](#cli-tools-usage)

​		[trustx_cert](#trustx_cert)

​		[trustx_chipinfo](#trustx_chipinfo)

​		[trustx_data](#trustx_data)

​		[trustx_keygen](#trustx_keygen)

​		[trustx_metadata](#trustx_metadata)

​		[trustx_read_app1](#trustx_read_app1)

​		[trustx_read_app2](#trustx_read_app2)

​		[trustx_read_cert](#trustx_read_cert)	

​		[trustx_readmetadata_app1](#trustx_readmetadata_app1)

​		[trustx_readmetadata_app2](#trustx_readmetadata_app2)

​		[trustx_readmetadata_cert](#trustx_readmetadata_cert)

​		[trustx_readmetadata_private](#trustx_readmetadata_private)

​		[trustx_readmetadata_status](#trustx_readmetadata_status)

​		[trustx_read_status](#trustx_read_status)

​		[trustx_sign](#trustx_sign)

​		[trustx_verify](#trustx_verify)

[Trust X OpenSSL Engine Usage](#trustx-openssl-engine-usage)

​		[rand](#rand)

​		[req](#req)

​		[pkey](#pkey)

​		[dgst](#dgst)

[Simple Example on OpenSSL using C language](#simple-example-on-openssl-using-c-language)

[Known Issues](#known-issues)

[License](#license)

------

## About

    This is a command line tools tools and OpenSSL Engine for OPTIGA Trust X on Linux platform.


## Prerequisites

    Following is the software component to build the tools :
    - GCC
    - OpenSSL development library (libssl-dev)
    - OpenSSL 1.1.X
    - OPTIGA Trust X library (source code)
    - pthread
    - rt
    - trustx library (source code)
    
    Following must be functional before the tools could be run :
    - Communication with Trust X via I2C
    
    Tested platforms:
      - Raspberry PI 3 on Linux kernel 4.19

## Contents of Package

    This tools consists of the following files & directory:
    .
    ├── bin		         /* all executable command and .so file (created during make)  */
    ├── LICENSE                           // MIT license file
    ├── linux_example                     // Source code for executable file
    │   ├── simpleTest_Client.c           // simple example for Client TLS/DTLS in C
    │   ├── simpleTest_Server.c           // simple example for Server TLS/DTLS in C
    │   ├── trustx_cert.c                 // read and store x.509 certificate in Trust X
    │   └── trustx_chipinfo.c             // list chip info
    │   ├── trustx_data.c                 // read and store raw data in Trust X
    │   ├── trustx_keygen.c               // Key generation 
    │   ├── trustx_metadata.c             // read and modify metadata of selected OID 
    │   ├── trustx_read_app1.c            // read all app1 data
    │   ├── trustx_read_app2.c            // read all app2 data
    │   ├── trustx_read_cert.c            // read all certificate OID data
    │   ├── trustx_readmetadata_app1.c    // read all metadata of app1
    │   ├── trustx_readmetadata_app2.c    // read all metadata of app2
    │   ├── trustx_readmetadata_cert.c    // read all metadata of certificate OID
    │   ├── trustx_readmetadata_private.c // read all metadata of keys OID
    │   ├── trustx_readmetadata_status.c  // read all metadata of status OID
    │   ├── trustx_read_status.c          // read all status data
    │   ├── trustx_sign.c                 // example of Trust X sign function
    │   └── trustx_verify.c               // example of Trust X verify function
    ├── Makefile                          // this project Makefile 
    ├── patch            /* patch folder for trustx library              */
    │   └── pal_os_event.c                // work around patch for trust X pal library
    ├── README.md                         // this read me file in Markdown format 
    ├── trustx_engine    /* all trust X OpenSSL Engine source code       */
    │   ├── trustx_engine.c               // entry point for Trust X OpenSSL Engine 
    │   ├── trustx_engine_common.h        // header file for Trust X OpenSSL Engine
    │   ├── trustx_engine_ec.c            // ECC engine api
    │   ├── trustx_engine_ec.h            // ECC engine header
    │   ├── trustx_engine_rand.c          // Random number generator  
    ├── trustx_helper    /* Helper rountine for trust X library           */
    │   ├── include	     /* Helper include directory                     
    │   │   └── trustx_helper.h	// Helper header file
    │   └── trustx_helper.c		// Helper source 
    └── trustx_lib       /* Directory for trust X library                 */

## Getting Started
### Before Building the tools

Before building the tools ensure that the [prerequisites](#prerequisites) is met.

#### Patching the trustx_lib

This patch is needed to work around known issue [sporadic hang or segment fault seem when using Trust X OpenSSL Engine](#sporadic-hang-or-segment-fault-seem-when-using-trust-x-openssl-engine)

Copy the file *pal_os_event.c* in the patch directory and over write the file with the same file name in Trust X library found in  ~/trustx_lib/pal/linux.

### First time building the library

As all the tools application depends on the libtrustx.so library hence it has to be build first else linking will fail due to missing library file.

There is 2 approach in building the Trust X library:

- Using the *install_debug_lib* parameter
  - this approach creates an symbolic link in the system library folder which is link to the bin folder of the project to libtrustx.so. 
  - This is useful for development and debugging as we do not need to keep updating the library folder which requires root access.
  - Need to perform this only once. 

```console 
foo@bar:~$ sudo make install_debug_lib
```
- Using the *install_lib* parameter
  - this approach copy the libtrustx.so into the system library folder. It will over write the symbolic link if it exist.
  - This is useful for trust X library deployment.
  - Need to execute every time the trust X library is updated.   

```console 
foo@bar:~$ sudo make install_lib
```

### Building the engine
```console 
foo@bar:~$ sudo make install_debug_engine
```
or 
```console 
foo@bar:~$ sudo make install_engine
```

Note:
- If debug is used subsequence building just do a *make* as the Makefile creates a 
  soft-link to the bin directory
- If without debug than every time you build the library or engine you must reinstall

### Build the command line tools

To build the command line tools just perform a *make*.

```console
foo@bar:~$ make
```

## CLI Tools Usage
### trustx_cert

Read/Write/Clear certificate from/to certificate data object. Output and input certificate in PEM format.

```console
foo@bar:~$ ./bin/trustx_cert
Help menu: trustx_cert <option> ...<option>
option:- 
-r <Cert OID>  	: Read Certificate from OID 0xNNNN 
-w <Cert OID>  	: Write Certificte to OID
-o <filename>  	: Output certificate to file 
-i <filename>  	: Input certificate to file 
-c <Cert OID>   : Clear cert OID data to zero 
-h              : Print this help 
```

Example to read OID 0xe0e0 and output the certification to teste0e0.crt 

```console
foo@bar:~$ ./bin/trustx_cert -r 0xe0e0 -o teste0e0.crt
===========================================
Reading OID 0xE0E0
Output File Name : teste0e0.crt 
Success!!!

foo@bar:~$ cat teste0e0.crt
-----BEGIN CERTIFICATE-----
MIIBuzCCAWKgAwIBAgIEEcNH6jAKBggqhkjOPQQDAjByMQswCQYDVQQGEwJERTEh
MB8GA1UECgwYSW5maW5lb24gVGVjaG5vbG9naWVzIEFHMRMwEQYDVQQLDApPUFRJ
R0EoVE0pMSswKQYDVQQDDCJJbmZpbmVvbiBPUFRJR0EoVE0pIFRydXN0IFggQ0Eg
MTAxMB4XDTE4MDUyNjA1NTUyNFoXDTM4MDUyNjA1NTUyNFowADBZMBMGByqGSM49
AgEGCCqGSM49AwEHA0IABJsmZ5A3QDUBLr17WBMeTKYW6yVCtod2mOiLaPZsccpY
bwIvkdNhaxApEKouSWYFsNRCp7o5PAdNWSnZzzMtrw+jWDBWMA4GA1UdDwEB/wQE
AwIAgDAMBgNVHRMBAf8EAjAAMBUGA1UdIAQOMAwwCgYIKoIUAEQBFAEwHwYDVR0j
BBgwFoAUygUz10/EfwlJ+9sSJd/Xl51BHhUwCgYIKoZIzj0EAwIDRwAwRAIgF2eA
aoyUOhuARXxjSTmTTn6/I6DutsH0vWS7qlRUQOECIFN5Mvq1DATJ6HPO0AkQsUdw
yKxbtETkDh3AyxdIbXrE
-----END CERTIFICATE-----
```

Example to write certificate teste0e0.crt into OID 0xe0e1

```console
foo@bar:~$ ./bin/trustx_cert -w 0xe0e1 -i teste0e0.crt
===========================================
Success!!!
```

Example to clear certificate store in OID 0xe0e1

```console
fop@bar:~$ ./bin/trustx_cert -c 0xe0e1
===========================================
Cleared.
```

### trustx_chipinfo

Display the Trust X chip information

```console
foo@bar:~$ ./bin/trustx_chipinfo
Read Chip Info [0xE0C2]: Success.
===========================================
CIM Identifier             [bCimIdentifer]: 0xcd
Platform Identifer   [bPlatformIdentifier]: 0x16
Model Identifer         [bModelIdentifier]: 0x33
ID of ROM mask                  [wROMCode]: 0x6b01
Chip Type                    [rgbChipType]: 0x00 0x1c 0x00 0x01 0x00 0x00
X-coordinate              [wChipPositionX]: 0x005c
Y-coordinate              [wChipPositionY]: 0x0046
Firmware Identifier [dwFirmwareIdentifier]: 0x80101071
Build Number                 [rgbESWBuild]: 11 18

Chip software build V1.40.1118
===========================================
```

### trustx_data

Read/Write/Erase OID data object in raw format.

```console
foo@bar:~$ ./bin/trustx_data
Help menu: trustx_data <option> ...<option>
option:- 
-r <OID>      : Read from OID 0xNNNN 
-w <OID>      : Write to OID
-i <filename> : Input file 
-o <filename> : Output file 
-p <offset>   : Offset position 
-e            : Erase and wirte 
-h            : Print this help
```

Example writing text file 1234.txt into OID 0xe0e1 and reading after writing

```console
foo@bar:~$ cat 1234.txt
1234

foo@bar:~$ ./bin/trustx_data -w 0xe0e1 -i 1234.txt 
Device Public Key           [0xE0E1] Offset: 0
Input data : 
	31 32 33 34 0a 
========================================================

foo@bar:~$ ./bin/trustx_data -r 0xe0e1
Device Public Key           [0xE0E1] [Size 0005] : 
	31 32 33 34 0a 
========================================================
```

Example erase with offset OID 0xe0e1

```console
foo@bar:~$ ./bin/trustx_data -w 0xe0e1 -e -p 10 -i 1234.txt 
========================================================
Device Public Key           [0xE0E1] Offset: 10
Input data : 
	31 32 33 34 0a 
Write Success.
========================================================

foo@bar:~$ ./bin/trustx_data -r 0xe0e1
Device Public Key           [0xE0E1] [Size 0015] : 
	00 00 00 00 00 00 00 00 00 00 31 32 33 34 0a 
========================================================
```

### trustx_keygen

Generate Trust X key pair. Key type can be or together to form multiple type.

```console

foo@bar:~$ ./bin/trustx_keygen 
Help menu: trustx_keygen <option> ...<option>
option:- 
-g <Key OID>    : Generate Key in OID 0xNNNN 
-t <key type>  	: Key type Auth:0x01 Enc :0x02 HFWU:0x04
                           DevM:0X08 Sign:0x10 Agmt:0x20
                           [default Auth]
-k <key size>   : Key size ECC256:0x03 ECC384:0x04 [default ECC256]
-o <filename>  	: Output Pubkey to file in PEM format
-h              : Print this help 
```

Example to generate an ECC256 key with type Auth, Enc and Sign in OID 0xe0f3.

```console
foo@bar:~$ ./bin/trustx_keygen -g 0xe0f3 -t 0x13 -k 0x03 -o teste0f3_pub.pem
Generating Key to 0xE0F3
Output File Name : teste0f3_pub.pem 
Pubkey :
	30 59 30 13 06 07 2a 86 48 ce 3d 02 01 06 08 2a 
	86 48 ce 3d 03 01 07 03 42 00 04 8a 3c 11 3c fa 
	9f 86 24 bf 48 2b 66 18 a2 68 fd 06 bc 24 32 94 
	10 a2 68 17 58 af 6a 93 6a a2 85 9a fc 08 0c 6a 
	15 f7 f4 73 97 2b ac f2 e5 87 f5 73 ff 22 8e 0b 
	8e 58 86 69 83 d2 41 99 95 8d 5b 
	
foo@bar:~$ cat teste0f3_pub.pem 
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEijwRPPqfhiS/SCtmGKJo/Qa8JDKU
EKJoF1ivapNqooWa/AgMahX39HOXK6zy5Yf1c/8ijguOWIZpg9JBmZWNWw==
-----END PUBLIC KEY-----
```

### trustx_metadata

Modify Trust X OID metadata.

***Warning : -L and -T option is not reversible. Lock set Lcs0 to 0x07 and Terminate set Lcs0 to 0x0F*** 

```console
foo@bar:~$ ./bin/trustx_metadata 
Help menu: trustx_metadata <option> ...<option>
option:- 
-r <OID>  : Read metadata of OID 0xNNNN 
-w <OID>  : Write metadata of OID
-C <data> : Set Change mode (a:allow change,
                             n:disable change,
                             t:disable change on termination,
                             f:<input file for complex setting>)
-R <data> : Set Read mode (a:allow read,
                           t:disable read on termination
                           f:<input file for complex setting>)
-L        : Lock OID metadata 
-T        : TERMINATE OID 
-h        : Print this help 
```

Example for changing OID 0xe0e1 metadata to read only and reading the metadata after changing

```console
foo@bar:~$ ./bin/trustx_metadata -w 0xe0e1 -Cn -Ra
Device Public Key           [0xE0E1] 
	20 06 D0 01 FF D1 01 00 
	C:NEV, R:ALW
Write Success.
========================================================

foo@bar:~$ ./bin/trustx_metadata -r 0xe0e1
========================================================
Device Public Key           [0xE0E1] [Size 0018] : 
	20 10 C0 01 01 C4 02 06 C0 C5 01 0F D0 01 FF D1 
	01 00 
	LcsO:0x01, Max:1728, Used:15, C:NEV, R:ALW
========================================================
```

Example for charging OID 0xe0e1 metadata using complex setting (LcsO>3||LcsG<4) for Change mode 

```console
foo@bar:~$ echo -e -n \\x07\\xe1\\xfb\\x03\\xfe\\x70\\xfc\\x04 > complexsetting.bin

foo@bar:~$ hd complexsetting.bin 
00000000  07 e1 fb 03 fe 70 fc 04                           |.....p..|
00000008

foo@bar:~$ ./bin/trustx_metadata -w 0xe0e1 -Cf:complexsetting.bin 
Device Public Key           [0xE0E1] 
	20 09 D0 07 E1 FB 03 FE 70 FC 04 
	C:LcsO>3||LcsG<4, 
========================================================

foo@bar:~$ ./bin/trustx_metadata -r 0xe0e1
Device Public Key           [0xE0E1] [Size 0024] : 
	20 16 C0 01 01 C4 02 06 C0 C5 01 0F D0 07 E1 FB 
	03 FE 70 FC 04 D1 01 00 
	LcsO:0x01, Max:1728, Used:15, C:LcsO>3||LcsG<4, R:ALW
========================================================
```

### trustx_read_app1

Read all app1 data object : 0xF1D0 - 0xF1DF

### trustx_read_app2

Read all app2 data object : 0xF1E0 - 0xF1E1

### trustx_read_cert

Read all certificate data object : 0xE0E0 - 0xF1E3, 0xE0E8, 0xE0EF

### trustx_readmetadata_app1

Read all app1 data object metadata : 0xF1D0 - 0xF1DF

### trustx_readmetadata_app2

Read all app2 data object metadata: 0xF1E0 - 0xF1E1

### trustx_readmetadata_cert

Read all certificate data object metadata : 0xE0E0 - 0xF1E3, 0xE0E8, 0xE0EF

### trustx_readmetadata_private

Read all keys data object metadata : 0xE0F0-0xE0F3

### trustx_readmetadata_status

Read all status data object metadata : 0xE0C0-0xE0C5, 0xF1C1-0xF1C2

### trustx_read_status

Read the data of status register : 0xE0C0-0xE0C5, 0xF1C1-0xF1C2

```console
foo@bar:~$ ./bin/trustx_read_status 
===========================================
Global Life Cycle Status    [0xE0C0] [Size 0001] : 07 
Global Security Status      [0xE0C1] [Size 0001] : 00 
UID                         [0xE0C2] [Size 0027] : 
	CD 16 33 6B 01 00 1C 00 01 00 00 0A 08 52 55 00 
Sleep Mode Activation Delay [0xE0C3] [Size 0001] : 14 
Current Limitation          [0xE0C4] [Size 0001] : 06 
Security Event Counter      [0xE0C5] [Size 0001] : 00 
Application Security Sts    [0xF1C1] [Size 0001] : 00 
Application Error Codes     [0xF1C2] [Size 0001] : 00 
===========================================
```

### trustx_sign

Simple demo to show the process to sign using Trust X key.

***Note : the option -H is not implemented. Input data is signed as it is, no hash is done.***

```console
foo@bar:~$ ./bin/trustx_sign 
Help menu: trustx_sign <option> ...<option>
option:- 
-k <OID Key>  : Select key for signing OID 0xNNNN 
-o <filename> : Output to file 
-i <filename> : Input Data file
-H            : Hash before sign
-h            : Print this help 
```

Example to sign the file helloworld.txt with key OID 0xE0F3 and output to testsignature.bin

```console
foo@bar:~$ ./bin/trustx_sign -k 0xe0f3 -o testsignature.bin -i helloworld.txt 
========================================================
OID Key 0xE0F3
Output File Name : testsignature.bin 
Input File Name : helloworld.txt 
Input data : 
	68 65 6C 6C 6F 77 6F 72 6C 64 0A 
Success
========================================================

foo@bar:~$ hd testsignature.bin 
00000000  02 21 00 a9 a3 39 36 8a  7f 9c 32 05 cb ea b6 41  |.!...96...2....A|
00000010  62 3f ea b0 78 3b 37 f4  3b 96 c1 68 3f aa b3 cf  |b?..x;7.;..h?...|
00000020  d3 71 75 02 21 00 e7 a6  6d 6a 76 88 81 01 f4 54  |.qu.!...mjv....T|
00000030  3e 03 34 2e 2a 6e 77 47  2b 0a fd fc 79 1e ac df  |>.4.*nwG+...y...|
00000040  6b 3f 5d 5c 77 52                                 |k?]\wR|
00000046
```

### trustx_verify

Simple demo to show the process to verify using Trust X library.

```console
foo@bar:~$ ./bin/trustx_verify 
Help menu: trustx_verify <option> ...<option>
option:- 
-k <OID Key>   : Read Certificate from OID 0xNNNN 
-i <filename>  : Input Data file
-s <signature> : Signature file
-p <pubkey>    : Host Pubkey
-H             : Hash before sign
-h             : Print this help 
```

Example for verifying a signature using external public key.

```console
foo@bar:~$ ./bin/trustx_verify -i helloworld.txt -s testsignature.bin -p teste0f3_pub.pem 
========================================================
Pubkey file : teste0f3_pub.pem
Input File Name : helloworld.txt 
Signature File Name : testsignature.bin 
	68 65 6C 6C 6F 77 6F 72 6C 64 0A 
Signature : 
	02 21 00 A9 A3 39 36 8A 7F 9C 32 05 CB EA B6 41 
	62 3F EA B0 78 3B 37 F4 3B 96 C1 68 3F AA B3 CF 
	D3 71 75 02 21 00 E7 A6 6D 6A 76 88 81 01 F4 54 
	3E 03 34 2E 2A 6E 77 47 2B 0A FD FC 79 1E AC DF 
	6B 3F 5D 5C 77 52 
Pub key : 
	03 42 00 04 8A 3C 11 3C FA 9F 86 24 BF 48 2B 66 
	18 A2 68 FD 06 BC 24 32 94 10 A2 68 17 58 AF 6A 
	93 6A A2 85 9A FC 08 0C 6A 15 F7 F4 73 97 2B AC 
	F2 E5 87 F5 73 FF 22 8E 0B 8E 58 86 69 83 D2 41 
	99 95 8D 5B 
Verify Success.
```

Example for verifying using certificate store in OID 0xE0E1

```console
foo@bar:~$ ./bin/trustx_verify -i helloworld.txt -s testsignaturee0e0.bin -k 0xe0e1
========================================================
OID Cert 0xE0E1
Input File Name : helloworld.txt 
Input data : 
	68 65 6C 6C 6F 77 6F 72 6C 64 0A 
Signature : 
	02 20 4C DA D4 B8 59 D0 A7 09 1F 39 8C C1 33 C8 
	27 25 01 1F F0 66 56 74 68 CA DE 19 51 D2 59 14 
	D6 DF 02 20 55 91 BF 8B 7A 50 AA 96 0A FD 3B FF 
	FD 13 61 66 9B FC D4 E6 E0 ED 5D F8 95 84 AC 48 
	A1 0A 68 8C 
Verify Success.
========================================================
```

## Trust X OpenSSL Engine usage

### rand
Usuage : Random number generation
Example
```console 
foo@bar:~$ openssl rand -engine trustx_engine -base64 1024
```
Note : 
If Trust X random number generation fails, there will still be random number output. 
This is control by OpenSSL. The engine api do not have control over it.

### req
Usuage : Certificate request / self signed cert / key generation.

Trust X engine uses the -key parameter to pass input to the key generation/usage function.

Following is the input format:

-key \<OID\> : \<public key input\> : \<NEW\> :\<key size\> : \<key usuage\>

where :

- OID for Trust X key
  - if OID 0xE0E0 is used no other input is needed 
- public key input:
  - public key file name in PEM format
  - \* = no public input
  - ^ = public key store in Application OID Key 
    - 0xE0F1 store in 0xE1D1,
    - 0xE0F2 store in 0xE1D2,
    - 0xE0F3 store in 0xE1D3
- NEW:
  - Generate new key pair in Trust X
- Key size
  - 0x03 = 256 key length
  - 0x04 = 384 key length
- Type
  - Auth:0x01 Enc :0x02 HFWU:0x04 DevM:0X08 Sign:0x10 Agmt:0x20

*Note: If wrong public is submitted the certificate generation will still go through but verification will fail.*

Example : Generating a certificate request using oid 0xE0F3 to sign and external public key. Verify that public key match the private key in the oid.

```console 
foo@bar:~$ openssl req -keyform engine -engine trustx_engine -key 0xe0f3:teste0f3_pub.pem -new -verify -out teste0e3.csr
```
Example : Generating a certificate request by generating a new key pair in Trust X in oid 0xE0F3. (using default key input of 256 length and auth)

```console 
foo@bar:~$ openssl req -keyform engine -engine trustx_engine -key 0xe0f3:*:NEW -new -out teste0e3.csr
```

Example : Generating a certificate request by generating a new 384 key pair, usage auth/enc/sign in Trust X in oid 0xE0F3 and store public key in 0xE1D3.

```console 
foo@bar:~$ openssl req -keyform engine -engine trustx_engine -key 0xe0f3:^:NEW:0x04:0x13 -new -out teste0e3.csr
```

### pkey

Usuage : Key tools / Key generation

Trust X engine uses the -in parameter to pass input to the key generation/usage function.

Following is the input format:

-in \<OID\> : \<public key input\> : \<NEW\> :\<key size\> : \<key usuage\>

(see [req](#req) for input details)

Example generate a new key pair in Trust X in oid 0xE0F3. (using default key input of 256 length and auth)

```console 
foo@bar:~$ openssl pkey -engine trustx_engine -pubout -inform engine -in 0xe0f3:*:NEW -out testpube0f3.pem
```

Example generate a new 384 key pair, usage auth/enc/sign in Trust X in oid 0xE0F3 and store public key in 0xE1D3.

```console 
foo@bar:~$ openssl pkey -engine trustx_engine -pubout -inform engine -in 0xe0f3:^:NEW:0x04:0x13 -out testpube0f3.pem
```

### dgst

Usuage : Sign and verify
Example to sign using oid 0xe0f3

```console 
foo@bar:~$ openssl dgst -sign 0xe0f3 -engine trustx_engine -keyform engine -out helloworld.sig helloworld.txt
```
Example to verify signature.

*Note : verify signature is not using Trust X engine.*

```console 
foo@bar:~$ openssl dgst -verify testpube0f3.pem -signature helloworld.sig helloworld.txt
```

## Simple Example on OpenSSL using C language

In this section, we will describe and demo how the Trust X OpenSSL engine could be coded in 'C' to perform TLD/DTLS communication.

*Note : The code only shows example on using Trust X for authentication and the secure communication is done via OpenSSL.*

### Setting up the environment for the demonstration

For easy setup, the demo uses the following input for :

- Server (system with Trust X and listening to connection)
  - server certificate : cert store in oid 0xE0E0
  - Trust X key : 0xE0F0
  - CA certificate :  OPTIGA_Trust_X_trusted_CAs.pem
  - Port : 5000
  - SSL Protocol : DTLS1.2
- Client (system that send HELLO request)
  - CA certificate :  OPTIGA_Trust_X_trusted_CAs.pem
  - IP : 127.0.0.1
  - Port : 5000
  - SSL Protocol : DTLS1.2

#### Server Setup

##### Getting the 0xE0E0 Certificate in Trust X and save it as teste0e0.crt

You may used the below example to get the cert.

```console
foo@bar:~$ ./bin/trustx_cert -r 0xe0e0 -o teste0e0.crt
===========================================
Reading OID 0xE0E0
Output File Name : teste0e0.crt 
Success!!!
```

##### CA Certificate 

Ensure *OPTIGA_Trust_X_trusted_CAs.pem* is in the same directory as teste0e0.crt.

The *OPTIGA_Trust_X_trusted_CAs.pem* contain 3 certificate namely:

- Infineon OPTIGA(TM) ECC Root CA
- Infineon Test Server Intermediate CA
- Infineon OPTIGA(TM) Trust X CA 101

Below is a quick tips for verifying the Server cert matches the CA cert with OpenSSL

```console
foo@bar:~$ openssl verify -CAfile OPTIGA_Trust_X_trusted_CAs.pem -show_chain teste0e0.crt 
teste0e0.crt: OK
Chain:
depth=0:  (untrusted)
depth=1: C = DE, O = Infineon Technologies AG, OU = OPTIGA(TM), CN = Infineon OPTIGA(TM) Trust X CA 101
depth=2: C = DE, O = Infineon Technologies AG, OU = OPTIGA(TM) Devices, CN = Infineon OPTIGA(TM) ECC Root CA
```

#### Client Setup

Ensure *OPTIGA_Trust_X_trusted_CAs.pem* is in the current directory.

### Running the demonstration

As the default IP is set to a loopback IP 127.0.0.1 both the server and client need to be run on the same system. To run the client and server on a different system please refer to [More about simpleTest_Server](#more-about-simpletest_server) and [More about simpleTest_Client](#more-about-simpletest_client)

To build the demo refer to [Build the command line tools](#build-the-command-line-tools)

#### Running the Server

Open a new terminal in the system and ensure *teste0e0.crt* and *OPTIGA_Trust_X_trusted_CAs.pem* is in the current folder. Run simpleTest_Server

Example of simpleTest_Server running without client connection

```console
foo@bar:~$ ./bin/simpleTest_Server 
97 main: *****************************************
149 serverListen: Listening to incoming connection
```

#### Running the Client

Open another new terminal in the system and ensure *OPTIGA_Trust_X_trusted_CAs.pem* is in the current folder. Run simpleTest_Client

Example of simpleTest_Client running with connection to server

```console
foo@bar:~$ ./bin/simpleTest_Client 
87 main: *****************************************
112 doClientConnect: s_ipaddr : 127.0.0.1
149 doClientConnect: Connecting to server ....
167 doClientConnect: Connected to 127.0.0.1, port :0x8813
189 doClientConnect: Performing Handshaking .....
198 doClientConnect: Connection using : ECDHE-ECDSA-AES256-GCM-SHA384
199 doClientConnect:                  : DTLSv1.2
From Server [1712] : 001
From Server [1712] : 002
From Server [1712] : 003
From Server [1712] : 004
From Server [1712] : 005
From Server [1712] : 006
```

Server terminal output

```console
foo@bar:~$ ./bin/simpleTest_Server 
97 main: *****************************************
149 serverListen: Listening to incoming connection
162 serverListen: Connection from 127.0.0.1, port :0x8e8f
149 serverListen: Listening to incoming connection
247 doServerConnected: Init Trustx Engine. Ok
266 doServerConnected: Load Certificate ok
274 doServerConnected: Private Key Match the Server Certificate.
283 doServerConnected: Load CA cert ok
306 doServerConnected: Performing Handshking ......... 
311 doServerConnected: Connection using : ECDHE-ECDSA-AES256-GCM-SHA384
312 doServerConnected:                  : DTLSv1.2
313 doServerConnected: ++++++++++++++++++++++++++++++++++++++++++++++
333 doServerConnected: [1712] Received : 1
333 doServerConnected: [1712] Received : 2
333 doServerConnected: [1712] Received : 3
333 doServerConnected: [1712] Received : 4
333 doServerConnected: [1712] Received : 5
333 doServerConnected: [1712] Received : 6
```

The above console screen show a successful server/client connection via DTLS1.2. After the DTLS handshake is completed the client will send count from 1 to 100 to the server. When server received the data from client it will is display the info received and send back the Process ID (PID) and data received to the client. The client when received the data from the service, it will display them on the screen.

To run multiple client connection, open another new terminal in the system and ensure *OPTIGA_Trust_X_trusted_CAs.pem* is in the current folder. Run simpleTest_Client. 

Below show an example of the server receiving from multiple client.

```console
foo@bar:~$ ./bin/simpleTest_Server 
97 main: *****************************************
149 serverListen: Listening to incoming connection
162 serverListen: Connection from 127.0.0.1, port :0x968f
149 serverListen: Listening to incoming connection
247 doServerConnected: Init Trustx Engine. Ok
253 doServerConnected: Set Default Engine Ok.
266 doServerConnected: Load Certificate ok
274 doServerConnected: Private Key Match the Server Certificate.
283 doServerConnected: Load CA cert ok
306 doServerConnected: Performing Handshking ......... 
311 doServerConnected: Connection using : ECDHE-ECDSA-AES256-GCM-SHA384
312 doServerConnected:                  : DTLSv1.2
313 doServerConnected: ++++++++++++++++++++++++++++++++++++++++++++++
333 doServerConnected: [1761] Received : 1
333 doServerConnected: [1761] Received : 2
162 serverListen: Connection from 127.0.0.1, port :0x988f
149 serverListen: Listening to incoming connection
241 doServerConnected: Engine ID : trustx_engine
247 doServerConnected: Init Trustx Engine. Ok
253 doServerConnected: Set Default Engine Ok.
266 doServerConnected: Load Certificate ok
274 doServerConnected: Private Key Match the Server Certificate.
283 doServerConnected: Load CA cert ok
306 doServerConnected: Performing Handshking ......... 
311 doServerConnected: Connection using : ECDHE-ECDSA-AES256-GCM-SHA384
312 doServerConnected:                  : DTLSv1.2
313 doServerConnected: ++++++++++++++++++++++++++++++++++++++++++++++
333 doServerConnected: [1763] Received : 1
333 doServerConnected: [1761] Received : 3
333 doServerConnected: [1763] Received : 2
333 doServerConnected: [1761] Received : 4
333 doServerConnected: [1763] Received : 3
333 doServerConnected: [1761] Received : 5
333 doServerConnected: [1763] Received : 4
333 doServerConnected: [1761] Received : 6
333 doServerConnected: [1763] Received : 5
162 serverListen: Connection from 127.0.0.1, port :0x9a8f
149 serverListen: Listening to incoming connection
241 doServerConnected: Engine ID : trustx_engine
247 doServerConnected: Init Trustx Engine. Ok
253 doServerConnected: Set Default Engine Ok.
266 doServerConnected: Load Certificate ok
274 doServerConnected: Private Key Match the Server Certificate.
283 doServerConnected: Load CA cert ok
306 doServerConnected: Performing Handshking ......... 
311 doServerConnected: Connection using : ECDHE-ECDSA-AES256-GCM-SHA384
312 doServerConnected:                  : DTLSv1.2
313 doServerConnected: ++++++++++++++++++++++++++++++++++++++++++++++
333 doServerConnected: [1766] Received : 1
333 doServerConnected: [1761] Received : 7
333 doServerConnected: [1763] Received : 6
333 doServerConnected: [1766] Received : 2
333 doServerConnected: [1761] Received : 8
333 doServerConnected: [1763] Received : 7
333 doServerConnected: [1766] Received : 3
```

### More about simpleTest_Server

```c
// Macro for Keys/Certificates
#define SERVER_CERT     "teste0e0.crt"
#define SERVER_KEY      "0xe0f0"
#define CA_CERT         "OPTIGA_Trust_X_trusted_CAs.pem"

// Macro for Engine
#define ENGINE_NAME     "trustx_engine"

// Default IP/PORT
#define DEFAULT_IP      "127.0.0.1"
#define DEFAULT_PORT    5000
//#define SECURE_COMM   TLS_server_method()
#define SECURE_COMM     DTLS_server_method()
```

In the *simpleTest_Server.c* code ~ line number 54-66. List the macro for changing following input:

- SERVER_CERT      *\<filename for server certificate in PEM format\>* 
- SERVER_KEY        *<OID of Trust X key used. Refer to [OpenSSL req](#req) for the key input format>*
- CA_CERT               *\<CA Certificate filename. if CA cert is chain ensure all cert is in the chain\>*
- ENGINE_NAME    *\<Trust X engine name\>*
- DEFAULT_IP         *\<IP address, not important for server\>*
- DEFAULT_PORT   *\<Port to use for connection\>*
- SECURE_COMM   *\<SSL Protocol to be used TLS/DTLS\>*

### More about simpleTest_Client

```c
// Macro for Keys/Certificates
#define CA_CERT      "OPTIGA_Trust_X_trusted_CAs.pem"

// Macro for Engine
#define ENGINE_NAME  "trustx_engine"

// Default IP/PORT
#define DEFAULT_IP    "127.0.0.1"
#define	DEFAULT_PORT  5000
//#define SECURE_COMM TLS_client_method()
#define SECURE_COMM   DTLS_client_method()
```

In the *simpleTest_Client.c* code ~ line number 53-63. List the macro for changing following input:

- CA_CERT               *\<CA Certificate filename. if CA cert is chain ensure all cert is in the chain\>*
- ENGINE_NAME    *\<Engine name. Not important for Client as Client is not using Trust X\>*
- DEFAULT_IP         *\<IP address of the server\>*
- DEFAULT_PORT   *\<Port to use for connection\>*
- SECURE_COMM   *\<SSL Protocol to be used TLS/DTLS\>*

## Known issues

### Unable to send GPIO signal in Raspberry without sudo

In the Trust X pal Linux library, you may encounter error "Failed to open gpio .......". This prevent the host from send a reset to the Trust X. To work around this used the sudo command.

Without resetting Trust X, it may fail at time and require to be reset or user needs to run the command again. 

### Sporadic hang or segment fault seem when using Trust X OpenSSL Engine

When sporadic hanging or segment fault is seem when using the Trust X OpenSSL engine (Especially after modification of the engine code). Ensure that the pal_os_event.c patch is implemented. In the engine code ensure that trustx_disarm_timer(); is call after any Trust X library API is used.

### At time displace may shown miss-align

Run the tools again to refresh the output

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details





