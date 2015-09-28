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

#include <vector>
#include <themis/themis.h>
#include "exception.hpp"

namespace themis{

    typedef const std::vector<uint8_t>& (*get_public_key_for_id_callback_t)(const std::string& id);

    class secure_session_callback_interface{
    public:
	virtual const std::vector<uint8_t> get_pub_key_by_id(const std::vector<uint8_t>& id)=0;
    };


    int get_public_key_for_id_callback(const void *id, size_t id_length, void *key_buffer, size_t key_buffer_length, void *user_data){
	std::vector<uint8_t> pubk=((secure_session_callback_interface*)user_data)->get_pub_key_by_id();
	if(pubk.size()>key_buffer_length){
	    return THEMIS_BUFFER_TOO_SMALL;
	}
	memcpy(key_buffer, &pubk[0], pubk.size());
	return THEMIS_SUCCESS;
    }

    class secure_session{
    public:
	secure_session(const std::string& id, const std::vector<uint8_t>& priv_key, secure_session_callback_interface* callbacks):
	    _session(NULL),
	    _callback(NULL, NULL, NULL, themis::get_public_key_for_id_callback, callbacks){
	    secure_session_user_callback_t 
	    _session=secure_session_create(id.c_str(), id.length(), &priv_key[0], priv_key.size(), _callback);
	}

	virtual ~secure_session(){
	}

    private:
	secure_session_t* _session;
	secure_session_user_callback_t _callback;
    };
}// ns themis