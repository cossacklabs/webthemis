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
#include "ppapi/cpp/websocket.h"

#include "base64.hpp"
#include "websocket_api.hpp"

#include "secure_cell.hpp"
#include "secure_session.hpp"
#include "secure_keygen.hpp"


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>



const std::string server_pub_key="VUVDMgAAAC11WDPUAhLfH+nqSBHh+XGOJBHL/cCjbtasiLZEwpokhO5QTD6g";

class callback: public themis::secure_session_callback_interface{
public:
    const std::vector<uint8_t> get_pub_key_by_id(const std::vector<uint8_t>& id){
	std::string id_str(&id[0], &id[0]+id.size());
        if(id_str=="server")
    	    return base64_decode(server_pub_key);
	return std::vector<uint8_t>(0);
    }
};


class secure_chat_client_instance : public pp::Instance, public webthemis::web_socket_receive_listener {
public:
    explicit secure_chat_client_instance(PP_Instance instance) : 
	pp::Instance(instance),
	client(NULL),
	socket(pp::InstanceHandle(this))
    {
	post("info", std::string("used themis version ")+themis_version());
	try{
	  themis::key_pair_generator_t<themis::EC> kpg;
	  client =  new themis::secure_session("aa", kpg.get_priv(), &callbacks);
	  client->init();
	}catch(themis::exception& e){
	  postError("can't initialize connection with server", e.what());
	}
	socket.connect();
	
    }
    virtual ~secure_chat_client_instance() {
	delete client;
    }

    virtual bool Init(uint32_t /*argc*/,
                    const char * /*argn*/ [],
                    const char * /*argv*/ []) {
	return true;
    }

public:
  void on_data_received(const std::string& data){
  }
  void on_error(){
  }
  void on_open(){
  }
  void on_close(){
  }
  
private:
  callback callbacks;
  themis::secure_session* client;
  pp::WebSocket socket;

  void post(const std::string& command, const std::string& param1, const std::string& param2=""){
    pp::VarArray message;
    message.Set(0,command);
    message.Set(1,param1);
    if(param2 != ""){
      message.Set(2,param2);
    }
    PostMessage(message);
  }

  void postError(const std::string& message, const std::string& details=""){
    post("error", message, details);
  }

  void postInfo(const std::string& message){
    post("info", message);
  }

public:
    virtual void HandleMessage(const pp::Var& var_message) {
	if (!var_message.is_string())
	    return;
	std::istringstream message(var_message.AsString());
	std::string operation;
	std::string data;
	message>>operation>>data;
	if (operation == "post"){
	  postInfo("data");
	}
	else{
	  postError("openation not supported");
	}
    }
};

class secure_chat_client_module : public pp::Module {
public:
    secure_chat_client_module() : pp::Module() {}
    virtual ~secure_chat_client_module() {}
    virtual pp::Instance* CreateInstance(PP_Instance instance) {
	return new secure_chat_client_instance(instance);
    }
};

namespace pp {
    Module* CreateModule() {
	return new secure_chat_client_module();
    }
} // namespace pp
