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


#include "themispp/secure_cell.hpp"
#include "helpers/base64.hpp"

#define UI_STRING_PARAM(array, i) array.Get(i).AsString()

#define STR_2_VEC(str) std::vector<uint8_t>((str).c_str(), (str).c_str()+(str).length())


namespace pnacl{

class secure_cell_instance : public pp::Instance {
 public:
  explicit secure_cell_instance(PP_Instance instance) : 
    pp::Instance(instance){
	nacl_io_init_ppapi(instance, pp::Module::Get()->get_browser_interface());
    }

  virtual ~secure_cell_instance() {}

  virtual void HandleMessage(const pp::Var& var_message) {
    if (!var_message.is_array()){
      PostMessage(std::string("non array messages not supported"));
      return;
    }
    const pp::VarArray params(var_message);
    try{
	if (UI_STRING_PARAM(params,0) == "encrypt"){
	    std::string password=UI_STRING_PARAM(params,1);
	    std::string message=UI_STRING_PARAM(params,2);
	    themispp::secure_cell_seal_t scs(STR_2_VEC(password));
	    std::string res=helpers::base64_encode(scs.encrypt(STR_2_VEC(message)));
	    PostMessage(pp::Var(res));
	} else if (UI_STRING_PARAM(params,0) == "decrypt"){
	    std::string password=UI_STRING_PARAM(params,1);
	    std::string message=UI_STRING_PARAM(params,2);
	    themispp::secure_cell_seal_t scs(STR_2_VEC(password));
	    std::vector<uint8_t> res=(scs.decrypt(helpers::base64_decode(message)));
	    PostMessage(pp::Var(std::string((const char*)(&res[0]), res.size())));
	} else
	    PostMessage(std::string("undefined command"));
    }catch(themispp::exception_t& e){
	PostMessage(e.what());
    }
  }
};

class secure_cell_module : public pp::Module {
 public:
  secure_cell_module() : pp::Module() {}
  virtual ~secure_cell_module() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new secure_cell_instance(instance);
  }
};

}
namespace pp {
    Module* CreateModule() {
        return new pnacl::secure_cell_module();
    }
}  // namespace pp
