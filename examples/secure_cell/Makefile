#
#  Copyright (c) 2015 Cossack Labs Limited
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

WARNINGS := -Wno-long-long -Wall -Wswitch-enum -pedantic -Werror
CXXFLAGS := -pthread $(WARNINGS)

#
# Compute tool paths
#
GETOS := python $(PNACL_ROOT)/tools/getos.py
OSHELPERS = python $(PNACL_ROOT)/tools/oshelpers.py
OSNAME := $(shell $(GETOS))
RM := $(OSHELPERS) rm

PNACL_TC_PATH := $(abspath $(PNACL_ROOT)/toolchain/$(OSNAME)_pnacl)
PNACL_CXX := $(PNACL_TC_PATH)/bin/pnacl-clang++
PNACL_FINALIZE := $(PNACL_TC_PATH)/bin/pnacl-finalize
CXXFLAGS := -I$(PNACL_ROOT)/include -I$(PNACL_ROOT)/include/pnacl -I../../themis/src/wrappers/themis -I../../themis/src
LDFLAGS := -L$(PNACL_ROOT)/lib/pnacl/Release -L../../build -lppapi_cpp -lnacl_io -lthemis -lsoter -lcrypto -lppapi --pnacl-exceptions=sjlj


all: secure_cell.pexe

clean:
	$(RM) secure_cell.pexe secure_cell.bc

secure_cell.bc: secure_cell.cc
	$(PNACL_CXX) -o $@ $< -O2 $(CXXFLAGS) $(LDFLAGS) getentropy_pnacl.cc

secure_cell.pexe: secure_cell.bc
	$(PNACL_FINALIZE) -o $@ $<


#
# Makefile target to run the SDK's simple HTTP server and serve this example.
#
HTTPD_PY := python $(PNACL_ROOT)/tools/httpd.py --no-dir-check

.PHONY: serve
serve: all
	$(HTTPD_PY) -C $(CURDIR)
