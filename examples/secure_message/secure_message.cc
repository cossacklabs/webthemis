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

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_array.h"
#include "nacl_io/nacl_io.h"


#include "themispp/secure_message.hpp"
#include "themispp/secure_keygen.hpp"
#include "helpers/base64.hpp"

#define UI_STRING_PARAM(array, i) array.Get(i).AsString()

#define STR_2_VEC(str) std::vector<uint8_t>((str).c_str(), (str).c_str()+(str).length())


namespace pnacl{

class secure_message_instance : public pp::Instance {
 public:
  explicit secure_message_instance(PP_Instance instance) : 
    pp::Instance(instance){
	nacl_io_init_ppapi(instance, pp::Module::Get()->get_browser_interface());
	themispp::secure_key_pair_generator_t<themispp::EC> gen;
	public_key_=gen.get_pub();
	private_key_=gen.get_priv();
    }

  virtual ~secure_message_instance() {}

  virtual bool Init(uint32_t argc, const char * argn[], const char * argv[]) {
    pp::VarArray res;
    res.Set(0, std::string("keypair"));
    res.Set(1, helpers::base64_encode(private_key_));
    res.Set(2, helpers::base64_encode(public_key_));
    PostMessage(res);
    return true;
  }

  virtual void HandleMessage(const pp::Var& var_message) {
    if (!var_message.is_array()){
      PostMessage(std::string("non array messages not supported"));
      return;
    }
    const pp::VarArray params(var_message);
    try{
	if (UI_STRING_PARAM(params,0) == "encrypt"){
	    std::string peer_public_key=UI_STRING_PARAM(params,1);
	    std::string message=UI_STRING_PARAM(params,2);
	    themispp::secure_message_t sm(private_key_, helpers::base64_decode(peer_public_key));
	    std::string res_str=helpers::base64_encode(sm.encrypt(STR_2_VEC(message)));
	    pp::VarArray res;
	    res.Set(0, std::string("result"));
	    res.Set(1, res_str);
	    PostMessage(res);
	} else if (UI_STRING_PARAM(params,0) == "decrypt"){
	    std::string peer_public_key=UI_STRING_PARAM(params,1);
	    std::string message=UI_STRING_PARAM(params,2);
	    themispp::secure_message_t sm(private_key_, helpers::base64_decode(peer_public_key));
	    std::vector<uint8_t> res_vec=(sm.decrypt(helpers::base64_decode(message)));
	    pp::VarArray res;
	    res.Set(0, std::string("result"));
	    res.Set(1, std::string((const char*)(&res_vec[0]), res_vec.size()));
	    PostMessage(res);
	} else
	    PostMessage(std::string("undefined command"));
    }catch(themispp::exception_t& e){
	PostMessage(e.what());
    }
  }
 private:
    std::vector<uint8_t> private_key_;
    std::vector<uint8_t> public_key_;
};

class secure_message_module : public pp::Module {
 public:
  secure_message_module() : pp::Module() {}
  virtual ~secure_message_module() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new secure_message_instance(instance);
  }
};

}
namespace pp {
    Module* CreateModule() {
        return new pnacl::secure_message_module();
    }
}  // namespace pp
