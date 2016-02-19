# webthemis
webthemis is PNaCl wrapper for Themis, it is now run as an experimental feature for internal testing

#intro

Native Client (NaCl) is a sandboxing technology for running a subset of Intel x86/x86-64, ARM or MIPSnative code in a sandbox. It allows safely running native code from a web browser, independent of the user operating system, allowing web-based applications to run at near-native speeds.

PNaCl solves the portability problem by splitting the compilation process into two parts:

1. compiling the source code to a bitcode executable (pexe), and
2. translating the bitcode to a host-specific executable as soon as the module loads in the browser but before any code execution.

# create pnacl object

## Download and install the Native Client SDK

Follow the instructions on the Download page to download and install the Native Client SDK.

## install webthemis

To install webthemis type in terminal
```
cd <your pnacl project folder>
git clone https://github.com/cossacklabs/webthemis
cd webthemis
make
```
after build complete you need add to your pnacl project:
```
CXXFLAGS+= -Iwebthemis/themis/src -Iwebthemis/themis/src/wrappers/themis webthemis/getentropy_pnacl.cc
LDFLACS+= -Lwebthemis/build -lthemis -lsoter -lcrypto -lnacl_io
```
!!!! important
Any cryptography computation need to use random numbers generators. To enable using `/dev/urandom` in your project You need ti initialise `nacl_io` library by adding
```
#include "nacl_io/nacl_io.h"
```
to header of Your main pnacl object file and
```
nacl_io_init_ppapi(instance, pp::Module::Get()->get_browser_interface()); 
```
to Your pnacl object instance constructor.

## Communication between JavaScript and Native Client modules

The Native Client programming model supports bidirectional communication between JavaScript and the Native Client module. Both sides can initiate and respond to messages. In all cases, the communication is asynchronous.

### communication on C++ side

most of communication is realised by `HandleMessage` function, for example:
```
virtual void HandleMessage(const pp::Var& var_message) {
    if (!var_message.is_string()){
       PostMessage("error unsupported format");
    }
    std::istringstream message(var_message.AsString());
    std::string operation, password, base64encoded_data;
    message>>operation>>password>>base64encoded_data;
    try{
       if(operation == "encrypt"){
           themis::secure_cell_seal encrypter(std::vector<uint8_t>(password.data(), password.data()+password.length()));
           PostMessage(std::string("encrypt:"+base64encode(encrypter.encrypt(data)));
       }
       else if (operation == "decrypt"){
           themis::secure_cell_seal decrypter(std::vector<uint8_t>(password.data(), password.data()+password.length()));
           PostMessage(std::string("decrypt "+decrypter.decrypt(base64decode(data))));
       } else {
           PostMessage("error unsupported operation");
       }
    }catch(themispp::exception& e){
       PostMessage(std::string("error "+ e.what()));
    }
}
```
This code realize `HandleMessage` function for Pnacl encrypt/decrypt object by Secure Cell in Seal mode
