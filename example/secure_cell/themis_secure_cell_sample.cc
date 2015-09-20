/*
* Copyright (c) 2015 Cossack Labs Limited
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <cstdio>
#include <string>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/ppb_file_io.h"
#include "ppapi/cpp/directory_entry.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/cpp/file_ref.h"
#include "ppapi/cpp/file_system.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/message_loop.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_array.h"
#include "ppapi/utility/completion_callback_factory.h"
#include "ppapi/utility/threading/simple_thread.h"


#include "secure_cell.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class themis_secure_cell_samle_instance : public pp::Instance {
public:
    explicit themis_secure_cell_samle_instance(PP_Instance instance) : 
	pp::Instance(instance),
        callback_factory_(this),
        file_system_(this, PP_FILESYSTEMTYPE_LOCALPERSISTENT),
        file_system_ready_(false),
        file_thread_(this)
    {
	post("info", std::string("used themis version ")+themis_version());
    }
    virtual ~themis_secure_cell_samle_instance() {}

    virtual bool Init(uint32_t /*argc*/,
                    const char * /*argn*/ [],
                    const char * /*argv*/ []) {
	file_thread_.Start();
	file_thread_.message_loop().PostWork(callback_factory_.NewCallback(&themis_secure_cell_samle_instance::OpenFileSystem));
	return true;
    }

    void OpenFileSystem(int32_t /* result */) {
	int32_t rv = file_system_.Open(1024 * 1024, pp::BlockUntilComplete());
	if (rv == PP_OK) {
	    file_system_ready_ = true;
	    post("info","file system READY");
	} else {
//	    ShowErrorMessage("Failed to open file system", rv);
	}
    }

private:
    pp::CompletionCallbackFactory<themis_secure_cell_samle_instance> callback_factory_;
    pp::FileSystem file_system_;
    bool file_system_ready_;
    pp::SimpleThread file_thread_;

public:
    virtual void HandleMessage(const pp::Var& var_message) {
	if (!var_message.is_string())
	    return;
	std::istringstream message(var_message.AsString());
	std::string operation;
	std::string login;
	std::string password;
	std::string data;
	message>>operation>>login>>password;
	if(operation == "load"){
	     file_thread_.message_loop().PostWork(callback_factory_.NewCallback(&themis_secure_cell_samle_instance::Load, std::string("/")+login+".dat"));
	}
	else if (operation == "save"){
	    message.ignore(1);
	    std::string data="";
	    char a[1024];
	    message.getline(a, 1024);
	    data+=a;
	    while(!(message.eof())){
		message.getline(a, 1024);
		(data+="\n")+=a;
	    }
	    file_thread_.message_loop().PostWork(callback_factory_.NewCallback(&themis_secure_cell_samle_instance::Save, std::string("/")+login+".dat", data));
	}
    }

    void post(const std::string& command, const std::string& param1){
	pp::VarArray message;
	message.Set(0,command);
	message.Set(1,param1);
	PostMessage(message);
    }

    void Save(int32_t /* result */,
            const std::string& file_name,
            const std::string& file_contents) {
	if (!file_system_ready_) {
	    post("ERROR", "File system is not open");
	    return;
	}
	pp::FileRef ref(file_system_, file_name.c_str());
	pp::FileIO file(this);

	int32_t open_result = file.Open(ref, PP_FILEOPENFLAG_WRITE | PP_FILEOPENFLAG_CREATE | PP_FILEOPENFLAG_TRUNCATE, pp::BlockUntilComplete());
	if (open_result != PP_OK) {
	    post("ERROR", "File open for write failed");
	    return;
	}

	if (!file_contents.empty()) {
	    if (file_contents.length() > INT32_MAX) {
		post("ERROR", "File too big");
		return;
	    }
	    int64_t offset = 0;
	    int32_t bytes_written = 0;
	    do {
		bytes_written = file.Write(offset, file_contents.data() + offset, file_contents.length(), pp::BlockUntilComplete());
		if (bytes_written > 0) {
		    offset += bytes_written;
		} else {
		    post("ERROR", "File write failed");
		    return;
		}
	    } while (bytes_written < static_cast<int64_t>(file_contents.length()));
	}
	int32_t flush_result = file.Flush(pp::BlockUntilComplete());
	if (flush_result != PP_OK) {
	    post("ERROR", "File fail to flush");
	    return;
	}
	post("info", file_name+" saved succesfully");
    }

    void Load(int32_t /* result */, const std::string& file_name) {
	if (!file_system_ready_) {
	    post("ERROR", "File system is not open");
	    return;
	}
	pp::FileRef ref(file_system_, file_name.c_str());
	pp::FileIO file(this);

	int32_t open_result = file.Open(ref, PP_FILEOPENFLAG_READ, pp::BlockUntilComplete());
	if (open_result == PP_ERROR_FILENOTFOUND) {
	    post("ERROR", "File not found");
	    return;
	} else if (open_result != PP_OK) {
	    post("ERROR", "File open for read failed");
	    return;
	}
	PP_FileInfo info;
	int32_t query_result = file.Query(&info, pp::BlockUntilComplete());
	if (query_result != PP_OK) {
	    post("ERROR", "File query failed");
	    return;
	}
	if (info.size > INT32_MAX) {
	    post("ERROR", "File too big");
	    return;
	}

	std::vector<char> data(info.size);
	int64_t offset = 0;
	int32_t bytes_read = 0;
	int32_t bytes_to_read = info.size;
	while (bytes_to_read > 0) {
	    bytes_read = file.Read(offset, &data[offset], data.size() - offset, pp::BlockUntilComplete());
	    if (bytes_read > 0) {
		offset += bytes_read;
		bytes_to_read -= bytes_read;
	    } else if (bytes_read < 0) {
		post("ERROR", "File read failed");
		return;
	    }
	}
	std::string string_data(data.begin(), data.end());
	post("DISP", string_data);
	post("info", file_name+" loaded succesfully");
    }
};

class themis_secure_cell_samle_module : public pp::Module {
public:
    themis_secure_cell_samle_module() : pp::Module() {}
    virtual ~themis_secure_cell_samle_module() {}
    virtual pp::Instance* CreateInstance(PP_Instance instance) {
	return new themis_secure_cell_samle_instance(instance);
    }
};

namespace pp {
    Module* CreateModule() {
	return new themis_secure_cell_samle_module();
    }
} // namespace pp