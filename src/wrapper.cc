#include "wrapper.h"

using namespace Napi;
using namespace ns3;
using namespace std;

Wrapper::Wrapper(const Napi::CallbackInfo& info) : ObjectWrap(info)/*, echoServer(9)*/ {
  if (info.Length() > 1) {
    auto options = info[1].As<Napi::Object>();
    if (options.Has("pcapPath")) {
      this->pcapPath = options.Get("pcapPath").As<Napi::String>().Utf8Value();
    }
  }
  Time::SetResolution (Time::NS);
  NS_LOG_COMPONENT_DEFINE ("NS3 Wrapper");
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
}

Napi::Value Wrapper::run(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Simulator::Run();
  return env.Null();
}

Napi::Value Wrapper::setLogger(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Expected logger").ThrowAsJavaScriptException();
    return env.Null();
  }
  logger = Napi::Reference<Napi::Function>::New(info[0].As<Napi::Function>(), 1);
  return env.Null();
}

Napi::Value Wrapper::setPcapPath(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Expected path").ThrowAsJavaScriptException();
    return env.Null();
  }
  pcapPath = info[0].As<Napi::String>().Utf8Value();
  //debug << "pcap path " << pcapPath << endl;
  return env.Null();
}

Napi::Value Wrapper::runFromConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  this->fromConfig(info);
  Simulator::Stop(Seconds(this->stopTime));
  Simulator::Run();
  Simulator::Destroy();
  this->clear();
  return env.Null();
}

void Wrapper::clear() {
  delete animePointer;
}

Napi::Function Wrapper::GetClass(Napi::Env env) {
  return DefineClass(env, "Instance", {
    Wrapper::InstanceMethod("runFromConfig", &Wrapper::runFromConfig),
    Wrapper::InstanceMethod("setPcapPath", &Wrapper::setPcapPath),
  });
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Napi::String name = Napi::String::New(env, "Instance");
  exports.Set(name, Wrapper::GetClass(env));
  return exports;
}

NODE_API_MODULE(addon, Init)
