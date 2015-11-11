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

#ifndef _WEBSOCKET_API_HPP_
#define _WEBSOCKET_API_HPP_


#include <string>
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_array_buffer.h"
#include "ppapi/utility/websocket/websocket_api.h"

namespace webthemis{
  class web_socket_receive_listener
  {
  public:
    virtual void on_data_received(const std::string& data) = 0;
    virtual void on_error() = 0;
    virtual void on_open() = 0;
    virtual void on_close() = 0;
  };

  class web_socket_api: protected pp::WebSocketAPI
  {
  public:
    web_socket_api(pp::Instance* ppinstance, web_socket_receive_listener* recv_listener)
      : pp::WebSocketAPI(ppinstance), receive_listener_(recv_listener), ppinstance_(ppinstance) {}
    virtual ~web_socket_api() {}

    bool is_connected() { return pp::WebSocketAPI::GetReadyState() == PP_WEBSOCKETREADYSTATE_OPEN; }
    void open(const std::string& url) { pp::WebSocketAPI::Connect(url, NULL, 0); }
    void close() { pp::WebSocketAPI::Close(PP_WEBSOCKETSTATUSCODE_NORMAL_CLOSURE, "bye"); }
    void sendData(const std::string& data) { pp::WebSocketAPI::Send(data); }

  protected:
    virtual void WebSocketDidOpen() {
      receive_listener_->on_open();
    }

    virtual void WebSocketDidClose(bool wasClean, uint16_t code, const pp::Var& reason) {
      receive_listener_->on_close();      
    }

    virtual void HandleWebSocketMessage(const pp::Var& message)
    {
      if (message.is_array_buffer()) {
	pp::VarArrayBuffer vararybuf(message);
	char *data = static_cast<char*>(vararybuf.Map());
	std::string datastr(data, data + vararybuf.ByteLength());
	vararybuf.Unmap();
	receive_listener_->on_data_received(datastr);
      } else { // is string
	receive_listener_->on_data_received(message.AsString());
      }
    }
    
    virtual void HandleWebSocketError() {
      receive_listener_->on_error();
    }

  private:
    web_socket_api(const web_socket_api&);
    web_socket_api& operator=(const web_socket_api&);

    web_socket_receive_listener* const receive_listener_;
    pp::Instance * const ppinstance_;
  };

} /*namespace webthemis*/
#endif /* _WEBSOCKET_API_HPP_ */
