#
# Copyright (c) 2015 Cossack Labs Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

NACL_SDK_ROOT ?= /home/andrey/progs/libs/nacl_sdk/pepper_44/

# Project Build flags
WARNINGS := -Wno-long-long -Wall -Wswitch-enum -pedantic -Werror
CXXFLAGS := -pthread -std=gnu++98 $(WARNINGS)

#
# Compute tool paths
#
GETOS := python $(NACL_SDK_ROOT)/tools/getos.py
OSHELPERS = python $(NACL_SDK_ROOT)/tools/oshelpers.py
OSNAME := $(shell $(GETOS))
RM := $(OSHELPERS) rm

PNACL_TC_PATH := $(abspath $(NACL_SDK_ROOT)/toolchain/$(OSNAME)_pnacl)
PNACL_CXX := $(PNACL_TC_PATH)/bin/pnacl-clang++
PNACL_FINALIZE := $(PNACL_TC_PATH)/bin/pnacl-finalize
CXXFLAGS := -I$(NACL_SDK_ROOT)/include -I$(NACL_SDK_ROOT)/include/pnacl -I/home/andrey/progs/libs/themis-pnacl/src -I/home/andrey/progs/libs/libressl-2.2.3/include -DTHEMIS_PNACL -DOPENSSL_NO_X509 -D__cplusplus
LDFLAGS := -L$(NACL_SDK_ROOT)/lib/pnacl/Release -lppapi_cpp -lppapi -lnacl_io --pnacl-exceptions=sjlj

all: libressl themis

libressl:
	mkdir -p build
ifeq (,$(wildcard build/libcrypto.a))
	echo "building libressl..."
	cd libressl && ./autogen.sh && patch -p1 <../libressl-2.2.5-patch && \
	CFLAGS="-I$(PNACL_ROOT)/include/pnacl -I$(PNACL_ROOT)/usr/include/pnacl -DSSIZE_MAX=LONG_MAX -DNO_SYSLOG" ./configure --disable-shared --without-pic --host=x86-32 && \
	CFLAGS="-I$(PNACL_ROOT)/include/pnacl -I$(PNACL_ROOT)/usr/include/pnacl -DSSIZE_MAX=LONG_MAX -DNO_SYSLOG" make
	mv libressl/crypto/.libs/libcrypto.a build
endif
	$(PNACL_ROOT)/toolchain/linux_pnacl/bin/pnacl-ranlib build/libcrypto.a


themis:
	mkdir -p build
ifeq (,$(wildcard build/libthemis.a))
	cd themis && patch -p1 <../themis-patch && make themis_static
	mv themis/build/libsoter.a build
	mv themis/build/libthemis.a build
endif
	$(PNACL_ROOT)/toolchain/linux_pnacl/bin/pnacl-ranlib build/libsoter.a
	$(PNACL_ROOT)/toolchain/linux_pnacl/bin/pnacl-ranlib build/libthemis.a

clean:
	$(RM) -rf build
	cd libressl && git reset --hard
	cd themis && git reset --hard