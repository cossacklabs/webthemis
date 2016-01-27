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

PNACL_ROOT ?= /home/andrey/progs/libs/nacl_sdk/pepper_44/

# Project Build flags
WARNINGS := -Wno-long-long -Wall -Wswitch-enum -pedantic -Werror
CXXFLAGS := -pthread -std=gnu++98 $(WARNINGS)

#
# Compute tool paths
#
GETOS := python $(PNACL_ROOT)/tools/getos.py
OSHELPERS = python $(PNACL_ROOT)/tools/oshelpers.py
OSNAME := $(shell $(GETOS))
RM := $(OSHELPERS) rm

PNACL_TC_PATH := $(abspath $(PNACL_ROOT)/toolchain/$(OSNAME)_pnacl)
PNACL_TOOLCHAIN=toolchain/$(OSNAME)_pnacl
PNACL_CXX := $(PNACL_TC_PATH)/bin/pnacl-clang++
PNACL_FINALIZE := $(PNACL_TC_PATH)/bin/pnacl-finalize
CXXFLAGS := -I$(PNACL_ROOT)/include -I$(PNACL_ROOT)/include/pnacl -I/home/andrey/progs/libs/themis-pnacl/src -I/home/andrey/progs/libs/libressl-2.2.3/include -DTHEMIS_PNACL -DOPENSSL_NO_X509 -D__cplusplus
LDFLAGS := -L$(PNACL_ROOT)/lib/pnacl/Release -lppapi_cpp -lppapi -lnacl_io --pnacl-exceptions=sjlj

all: libressl_ themis_

libressl_:
	mkdir -p build
ifeq (,$(wildcard build/libcrypto.a))
	echo "building libressl..."
	cd libressl && ./autogen.sh && patch -p1 <../libressl-2.2.5-patch && \
	CC=$(PNACL_TC_PATH)/bin/pnacl-clang CXX=$(PNACL_TC_PATH)/bin/pnacl-clang++ LD=$(PNACL_TC_PATH)/bin/pnacl-ld AR=$(PNACL_TC_PATH)/bin/pnacl-ar CFLAGS="-I$(PNACL_ROOT)/include/pnacl -I$(PNACL_ROOT)/usr/include/pnacl -DSSIZE_MAX=LONG_MAX -DNO_SYSLOG" ./configure --disable-shared --without-pic --host=x86-32 && \
	CC=$(PNACL_TC_PATH)/bin/pnacl-clang CXX=$(PNACL_TC_PATH)/bin/pnacl-clang++ LD=$(PNACL_TC_PATH)/bin/pnacl-ld AR=$(PNACL_TC_PATH)/bin/pnacl-ar CFLAGS="-I$(PNACL_ROOT)/include/pnacl -I$(PNACL_ROOT)/usr/include/pnacl -DSSIZE_MAX=LONG_MAX -DNO_SYSLOG" make
	mv libressl/crypto/.libs/libcrypto.a build
endif
	$(PNACL_ROOT)/toolchain/linux_pnacl/bin/pnacl-ranlib build/libcrypto.a


themis_:
	mkdir -p build
ifeq (,$(wildcard build/libthemis.a))
	cd themis && patch -p1 <../themis-patch &&\
	PNACL_ROOT=$(PNACL_ROOT) PNACL_TOOLCHAIN=$(PNACL_TOOLCHAIN) make themis_static
	mv themis/build/libsoter.a build
	mv themis/build/libthemis.a build
endif
	$(PNACL_ROOT)/toolchain/linux_pnacl/bin/pnacl-ranlib build/libsoter.a
	$(PNACL_ROOT)/toolchain/linux_pnacl/bin/pnacl-ranlib build/libthemis.a

clean:
	$(RM) -rf build
	cd libressl && git clean -xfd && git reset --hard
	cd themis && git clean -xfd && git reset --hard

.PHONY: all