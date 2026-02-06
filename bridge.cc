#include <stdint.h>
#include <nan.h>

#include "bridge.h"
#include "rdp.h"

using v8::Function;
using v8::Local;
using v8::Number;
using v8::Value;
using v8::String;
using v8::Handle;
using v8::Array;
using Nan::Callback;
using Nan::HandleScope;
using Nan::New;
using Nan::MaybeLocal;
using Nan::Null;
using Nan::To;

NAN_METHOD(Connect) {
  Nan::HandleScope scope;
  Handle<Value> val;

  Handle<Array> jsArray = Handle<Array>::Cast(info[0]);
  char** cstrings = new char*[jsArray->Length()];

  for (unsigned int i = 0; i < jsArray->Length(); i++) {
    val = Nan::Get(jsArray, i).ToLocalChecked();
    std::string current = std::string(*Nan::Utf8String(val));
    cstrings[i] = new char[current.size() + 1];
    std::strcpy(cstrings[i], current.c_str());
  }

  Callback *callback = new Callback(info[1].As<Function>());

  int session_index = node_freerdp_connect(jsArray->Length(), cstrings, callback);
  info.GetReturnValue().Set(session_index);
}

NAN_METHOD(SendKeyEventScancode) {
  Nan::HandleScope scope;

  int session_index = Nan::To<uint32_t>(info[0]).FromMaybe(0);
  int scanCode = Nan::To<uint32_t>(info[1]).FromMaybe(0);
  int pressed = Nan::To<uint32_t>(info[2]).FromMaybe(0);

  node_freerdp_send_key_event_scancode(session_index, scanCode, pressed);
}

NAN_METHOD(SendPointerEvent) {
  Nan::HandleScope scope;

  int session_index = Nan::To<uint32_t>(info[0]).FromMaybe(0);
  int flags = Nan::To<uint32_t>(info[1]).FromMaybe(0);
  int x = Nan::To<uint32_t>(info[2]).FromMaybe(0);
  int y = Nan::To<uint32_t>(info[3]).FromMaybe(0);

  node_freerdp_send_pointer_event(session_index, flags, x, y);
}


NAN_METHOD(SetClipboard) {
  Nan::HandleScope scope;

  int session_index = Nan::To<uint32_t>(info[0]).FromMaybe(0);
  std::string str = std::string(*Nan::Utf8String(info[1]));

  node_freerdp_set_clipboard(session_index, (void*)str.c_str(), str.size());
}

NAN_METHOD(Close) {
  Nan::HandleScope scope;

  int session_index = Nan::To<uint32_t>(info[0]).FromMaybe(0);

  node_freerdp_close(session_index);
}
