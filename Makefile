#/**
#* MIT License
#*
#* Copyright (c) 2019 Infineon Technologies AG
#*
#* Permission is hereby granted, free of charge, to any person obtaining a copy
#* of this software and associated documentation files (the "Software"), to deal
#* in the Software without restriction, including without limitation the rights
#* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#* copies of the Software, and to permit persons to whom the Software is
#* furnished to do so, subject to the following conditions:
#*
#* The above copyright notice and this permission notice shall be included in all
#* copies or substantial portions of the Software.
#*
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#* SOFTWARE
#
#*/

TRUSTX = trustx_lib


LIBDIR =  $(TRUSTX)/pal/linux
LIBDIR += $(TRUSTX)/optiga/util
#LIBDIR += $(TRUSTX)/optiga/dtls
LIBDIR += $(TRUSTX)/optiga/crypt
LIBDIR += $(TRUSTX)/optiga/comms
LIBDIR += $(TRUSTX)/optiga/common
LIBDIR += $(TRUSTX)/optiga/cmd
LIBDIR += trustx_helper

#OTHDIR = $(TRUSTX)/examples/optiga
#OTHDIR += $(TRUSTX)/examples/ecdsa_utils
#OTHDIR += $(TRUSTX)/examples/authenticate_chip
#OTHDIR += $(TRUSTX)/examples/mbedtls_port
#OTHDIR += $(TRUSTX)/externals/mbedtls-2.12.0
 
BINDIR = bin
APPDIR = linux_example
ENGDIR = trustx_engine
LIB_INSTALL_DIR = /usr/lib/arm-linux-gnueabihf
ENGINE_INSTALL_DIR = $(LIB_INSTALL_DIR)/engines-1.1

INCDIR = $(TRUSTX)/optiga/include
INCDIR += $(TRUSTX)/optiga/include/optiga
INCDIR += $(TRUSTX)/optiga/include/optiga/ifx_i2c
INCDIR += $(TRUSTX)/optiga/include/optiga/dtls
INCDIR += $(TRUSTX)/optiga/include/optiga/comms
INCDIR += $(TRUSTX)/optiga/include/optiga/common
INCDIR += $(TRUSTX)/optiga/include/optiga/cmd
INCDIR += $(TRUSTX)/optiga/include/optiga/pal
INCDIR += $(TRUSTX)/pal/linux
INCDIR += $(TRUSTX)/externals/mbedtls-2.12.0/include
INCDIR += trustx_helper/include
INCDIR += trustx_engine
INCDIR += $(TRUSTX)/examples/ecdsa_utils

ifdef INCDIR
INCSRC := $(shell find $(INCDIR) -name '*.h')
INCDIR := $(addprefix -I ,$(INCDIR))
endif

ifdef LIBDIR
	LIBSRC := $(shell find $(LIBDIR) -name '*.c') 
	LIBOBJ := $(patsubst %.c,%.o,$(LIBSRC))
	LIB = libtrustx.so
endif

ifdef OTHDIR
	OTHSRC := $(shell find $(OTHDIR) -name '*.c')
	OTHOBJ := $(patsubst %.c,%.o,$(OTHSRC))
endif

ifdef APPDIR
	APPSRC := $(shell find $(APPDIR) -name '*.c')
	APPOBJ := $(patsubst %.c,%.o,$(APPSRC))
	APPS := $(patsubst %.c,%,$(APPSRC))
endif

ifdef ENGDIR
	ENGSRC := $(shell find $(ENGDIR) -name '*.c')
	ENGOBJ := $(patsubst %.c,%.o,$(ENGSRC))
	ENG = trustx_engine.so
endif

CC = gcc
DEBUG = -g

CFLAGS += -c  
#CFLAGS += $(DEBUG)
CFLAGS += $(INCDIR) 
CFLAGS += -Wall 
CFLAGS += -DENGINE_DYNAMIC_SUPPORT
#CFLAGS += -DMODULE_ENABLE_DTLS_MUTUAL_AUTH

LDFLAGS += -lrt 
LDFLAGS += -lpthread
LDFLAGS += -lssl
LDFLAGS += -lcrypto

LDFLAGS_1 = -ltrustx

all : $(BINDIR)/$(LIB) $(APPS) $(BINDIR)/$(ENG)

$(BINDIR)/$(ENG): %: $(ENGOBJ) $(INCSRC) $(BINDIR)/$(LIB)
	@echo "******* Linking $@ "
	@mkdir -p bin
	@$(CC) $(LDFLAGS) $(LDFLAGS_1) $(ENGOBJ) -shared -o $@

$(APPS): %: $(OTHOBJ) $(INCSRC) %.o
	@echo "******* Linking $@ "
	@mkdir -p bin
	@$(CC) $(LDFLAGS) $(LDFLAGS_1) $@.o $(OTHOBJ) -o $@
	@cp $@ bin/.

$(BINDIR)/$(LIB): %: $(LIBOBJ) $(INCSRC)
	@echo "******* Linking $@ "
	@mkdir -p bin
	@$(CC) $(LDFLAGS) $(LIBOBJ) -shared -o $@

$(LIBOBJ): %.o: %.c $(INCSRC)
	@echo "+++++++ Generating lib object: $< "
	@$(CC) $(CFLAGS) $< -o $@

%.o: %.c $(INCSRC)
	@echo "------- Generating application objects: $< "
	@$(CC) $(CFLAGS) $< -o $@

.Phony : clean install uninstall test
clean :
	@echo "Removing *.o from $(LIBDIR)" 
	@rm -rf $(LIBOBJ)
	@echo "Removing *.o from $(OTHDIR)" 
	@rm -rf $(OTHOBJ)
	@echo "Removing *.o from $(APPDIR)"
	@rm -rf $(APPOBJ)
	@echo "Removing *.o from $(ENGDIR)"
	@rm -rf $(ENGOBJ)
	@echo "Removing all application from $(APPDIR)"	
	@rm -rf $(APPS)
	@echo "Removing all application from $(BINDIR)"	
	@rm -rf bin/*

install_debug_lib: uninstall_lib $(BINDIR)/$(LIB)
	@echo "Create debug link library $(LIB_INSTALL_DIR)/$(LIB)"	
	@ln -s $(realpath $(BINDIR)/$(LIB)) $(LIB_INSTALL_DIR)/$(LIB)

install_lib: uninstall_lib $(BINDIR)/$(LIB)
	@echo "$(LIB_INSTALL_DIR)/$(LIB)"	
	@cp $(BINDIR)/$(LIB) $(LIB_INSTALL_DIR)/$(LIB)

install_debug_engine: uninstall_engine $(BINDIR)/$(ENG)
	@echo "Create debug link library $(ENGINE_INSTALL_DIR)/$(ENG)"	
	@ln -s $(realpath $(BINDIR)/$(ENG)) $(ENGINE_INSTALL_DIR)/$(ENG)

install_engine: uninstall_engine $(BINDIR)/$(ENG) 
	@echo "Installing library : $(ENGINE_INSTALL_DIR)/$(ENG)"	
	@mkdir -p $(ENGINE_INSTALL_DIR)
	@cp $(BINDIR)/$(ENG) $(ENGINE_INSTALL_DIR)/$(ENG)

uninstall_engine:
	@echo "Removing library : $(ENGINE_INSTALL_DIR)/$(ENG)"	
	@rm -f $(ENGINE_INSTALL_DIR)/$(ENG)

uninstall_lib:
	@echo "Removing library : $(LIB_INSTALL_DIR)/$(LIB)"	
	@rm -f $(LIB_INSTALL_DIR)/$(LIB)
	
