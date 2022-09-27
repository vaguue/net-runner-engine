#include "wrapper.h"

using namespace Napi;
using namespace ns3;
using namespace std;

#define int uint32_t

extern struct DebugStream debug;

pair<string, int> ipPort(string s) {
  string ip = "";
  int port = 0;
  int i = 0;
  bool hasIP = false;
  for (int k = 0; k < s.size(); ++k) {
    if (s[k] == ':') {
      hasIP = true;
      break;
    }
  }
  if (hasIP) {
    for (; i < s.size(); ++i) {
      if (s[i] == ':') {
        ++i;
        break;
      }
      else {
        ip += s[i];
      }
    }
  }
  istringstream(s.substr(i)) >> port;
  return make_pair(ip, port);
}

struct JsApp : public Application {
  JsApp();
  static TypeId GetTypeId(void);
  virtual ~JsApp ();
  virtual void StartApplication(void);
  virtual void StopApplication(void);
  void tick();
  void callOnTick();

  const Napi::CallbackInfo* info = nullptr;
  Napi::Function onTick;
  Napi::Function premadeCallback;
  Ptr<Socket> socket;
  void setup(const Napi::CallbackInfo* info, Ptr<Socket> socket, Address address, Napi::Function onTick, string interval);
  Address peer;
  EventId sendEvent;
  bool running;
  string interval = "0.1s";
  int microSeconds = 0;
};

JsApp::JsApp() : socket(0), peer(), sendEvent(), running(false) {}
JsApp::~JsApp() { socket = 0; }
TypeId JsApp::GetTypeId (void) {
  static TypeId tid = TypeId("JsApp")
    .SetParent<Application>()
    .SetGroupName("Net-runner-engine")
    .AddConstructor<JsApp>()
    ;
  return tid;
}
void JsApp::setup(const Napi::CallbackInfo* info, Ptr<Socket> socket, Address address, Napi::Function onTick, string interval) {
  this->onTick = onTick;
  this->interval = interval;
  this->info = info;
  this->socket = socket;
  this->peer = address;
  function<void(const Napi::CallbackInfo& info)> sendPacket = [this](const Napi::CallbackInfo& info) {
    if (info.Length() > 0) {
      auto buf = info[0].As<Napi::Buffer<uint8_t>>();
      Ptr<Packet> pkt = Create<Packet>(reinterpret_cast<const uint8_t*>(buf.Data()), buf.Length());
      this->socket->Send(pkt);
    }
  };
  premadeCallback = Napi::Function::New(info->Env(), sendPacket);
}
void JsApp::StartApplication (void) {
  running = true;
  if (InetSocketAddress::IsMatchingType(peer)) {
    socket->Bind();
  }
  else {
    socket->Bind6();
  }
  socket->Connect(peer);
  tick();
}
void JsApp::StopApplication (void) {
  running = false;
  if (sendEvent.IsRunning()) {
    Simulator::Cancel(sendEvent);
  }
  if (socket) {
    socket->Close();
  }
}
void JsApp::tick() {
  if (running) {
    sendEvent = Simulator::Schedule(Time(interval), &JsApp::callOnTick, this);
  }
}
void JsApp::callOnTick() {
  Napi::Env env = info->Env();
  Napi::Object arg = Napi::Object::New(env);
  arg.Set("time", Simulator::Now().GetMilliSeconds());
  arg.Set("sendPacket", premadeCallback);
  onTick.Call({arg});
  tick();
}

struct PingInfo {
  string interval = "0.5s";
  string dst;
  PingInfo(const Napi::Object& init) {
    dst = init.Get("dst").As<Napi::String>();
    if (init.Has("interval")) {
      interval = init.Get("interval").As<Napi::String>();
    }
  };
  void install(ApplicationContainer& apps, MyNode& v) {
    V4PingHelper ping = V4PingHelper(Ipv4Address(dst.c_str()));
    ping.SetAttribute ("Interval", TimeValue(Time(interval)));
    apps = ping.Install(v.node.Get(0));
  }
};

struct SinkInfo {
  InetSocketAddress dst = InetSocketAddress(Ipv4Address::GetAny());
  SinkInfo(const Napi::Object& init) {
    if (init.Has("dst")) {
      dst = InetSocketAddress(Ipv4Address(string(init.Get("dst").As<Napi::String>()).c_str()));
    }
  };
  void install(ApplicationContainer& apps, MyNode& v) {
    PacketSinkHelper sink = PacketSinkHelper ("ns3::Ipv4RawSocketFactory", dst);
    apps = sink.Install (v.node.Get(0));
  }
};

struct UdpServerInfo {
  string addr = "";
  int port;
  bool echo = false;
  UdpServerInfo(const Napi::Object& init, const Napi::CallbackInfo& info) {
    if (init.Get("dst").IsNumber()) {
      port = init.Get("dst").As<Napi::Number>().Uint32Value();
    }
    else {
      string dst = init.Get("dst").As<Napi::String>();
      auto ipp = ipPort(dst);
      addr = ipp.first;
      port = ipp.second;
    }
    echo = init.Has("echo");
  };
  void install(ApplicationContainer& apps, MyNode& v) {
    auto resIp = addr.size() > 0 ? Ipv4Address(addr.c_str()) : Ipv4Address::GetAny();
    if (echo) {
      UdpEchoServerHelper echoServer(port);
      apps = echoServer.Install(v.node.Get(0));
    }
    else {
      PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(resIp, port)));
      apps = sink.Install(v.node.Get(0));
    }
  }
};

struct TcpServerInfo {
  string addr = "";
  int port;
  TcpServerInfo(const Napi::Object& init, const Napi::CallbackInfo& info) {
    if (init.Get("dst").IsNumber()) {
      port = init.Get("dst").As<Napi::Number>().Uint32Value();
    }
    else {
      string dst = init.Get("dst").As<Napi::String>();
      auto ipp = ipPort(dst);
      addr = ipp.first;
      port = ipp.second;
    }
  }
  void install(ApplicationContainer& apps, MyNode& v) {
    auto resIp = addr.size() > 0 ? Ipv4Address(addr.c_str()) : Ipv4Address::GetAny();
    Address sinkLocalAddress (InetSocketAddress (resIp, port));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
    apps = sinkHelper.Install (v.node.Get(0));
  }
};

struct UdpClientInfo {
  string addr;
  int port;

  int maxBytes = 0;
  //int maxPackets = 20;
  int packetSize = 256;
  string dataRate = "5Mbps";
  bool bulk = false;

  UdpClientInfo(const Napi::Object& init, const Napi::CallbackInfo& info) {
    string dst = init.Get("dst").As<Napi::String>();
    auto ipp = ipPort(dst);
    addr = ipp.first;
    port = ipp.second;
    /*if (init.Has("maxPackets")) {
      maxPackets = init.Get("maxPackets").As<Napi::Number>().Uint32Value();
    }*/
    if (init.Has("packetSize")) {
      packetSize = init.Get("packetSize").As<Napi::Number>().Uint32Value();
    }
    if (init.Has("dataRate")) {
      dataRate = init.Get("dataRate").As<Napi::String>();
    }
    if (init.Has("maxBytes")) {
      maxBytes = init.Get("maxBytes").As<Napi::Number>().Uint32Value();
    }
    bulk = init.Has("bulk");
  }
  void install(ApplicationContainer& apps, MyNode& v) {
    if (bulk) {
      BulkSendHelper source("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address (addr.c_str()), port));
      source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
      apps = source.Install(v.node.Get(0));
    }
    else {
      OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address(addr.c_str()), port)));
      onoff.SetAttribute("PacketSize", UintegerValue(packetSize));
      onoff.SetAttribute("MaxBytes", UintegerValue(maxBytes));
      onoff.SetAttribute("DataRate", StringValue(dataRate));
      apps = onoff.Install(v.node.Get(0));
      /*UdpEchoClientHelper echoClient (Ipv4Address(addr.c_str()), port);
      echoClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
      echoClient.SetAttribute("PacketSize", UintegerValue(packetSize));
      apps = echoClient.Install (v.node.Get(0));*/
    }
  }
};

struct TcpClientInfo {
  const Napi::CallbackInfo& info;
  string addr = "";
  int port;

  int maxBytes = 0;
  int packetSize = 256;
  string dataRate = "5Mbps";
  bool bulk = false;

  Napi::Function onTick;
  string tickInterval = "";
  bool useTicks = false;

  TcpClientInfo(const Napi::Object& init, const Napi::CallbackInfo& info) : info{info} {
    string dst = init.Get("dst").As<Napi::String>();
    auto ipp = ipPort(dst);
    addr = ipp.first;
    port = ipp.second;
    if (init.Has("packetSize")) {
      packetSize = init.Get("packetSize").As<Napi::Number>().Uint32Value();
    }
    if (init.Has("dataRate")) {
      dataRate = init.Get("dataRate").As<Napi::String>();
    }
    if (init.Has("maxBytes")) {
      maxBytes = init.Get("maxBytes").As<Napi::Number>().Uint32Value();
    }
    if (init.Has("onTick") && init.Get("onTick").IsFunction() 
        && init.Has("tickInterval") && init.Get("tickInterval").IsString()) {
      useTicks = true;
      onTick = init.Get("onTick").As<Napi::Function>();
      tickInterval = init.Get("tickInterval").As<Napi::String>();
    }
  }
  void install(ApplicationContainer& apps, MyNode& v) {
    if (useTicks) {
      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(v.node.Get(0), TcpSocketFactory::GetTypeId());
      Ptr<JsApp> app = CreateObject<JsApp>();
      app->setup(&info, ns3TcpSocket, Address(InetSocketAddress(Ipv4Address(addr.c_str()), port)), onTick, tickInterval);
      v.node.Get(0)->AddApplication(app);
      apps.Add(app);
    } 
    else if (bulk) {
      BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address (addr.c_str()), port));
      source.SetAttribute("MaxBytes", UintegerValue (maxBytes));
      apps = source.Install(v.node.Get(0));
    }
    else {
      OnOffHelper onoff("ns3::TcpSocketFactory", Address(InetSocketAddress(Ipv4Address(addr.c_str()), port)));
      onoff.SetAttribute("PacketSize", UintegerValue(packetSize));
      onoff.SetAttribute("MaxBytes", UintegerValue(maxBytes));
      onoff.SetAttribute("DataRate", StringValue(dataRate));
      apps = onoff.Install(v.node.Get(0));
    }
  }
};

void setupApplications(NodeCont& myNodes, const options& opts, const Napi::CallbackInfo& info) {
  for (auto& keyVal : myNodes) {
    auto v = keyVal.second;
    for (auto e : v.apps) {
      ApplicationContainer apps;
      if (e.type == "ping") {
        PingInfo appInfo{e.init};
        appInfo.install(apps, v);
      }
      else if (e.type == "sink") {
        SinkInfo appInfo{e.init};
        appInfo.install(apps, v);
      }
      else if (e.type == "udp-server") {
        UdpServerInfo appInfo{e.init, info};
        appInfo.install(apps, v);
      }
      else if (e.type == "udp-client") {
        UdpClientInfo appInfo{e.init, info};
        appInfo.install(apps, v);
      }
      else if (e.type == "tcp-server") {
        TcpServerInfo appInfo{e.init, info};
        appInfo.install(apps, v);
      }
      else if (e.type == "tcp-client") {
        TcpClientInfo appInfo{e.init, info};
        appInfo.install(apps, v);
      }
      apps.Start (Time (e.start));
      if (e.stop != "end") {
        apps.Stop (Time (e.stop));
      }
      //apps.Stop (Seconds (e.stop > 0 ? e.stop : opts.animeLen));
    }
  }
}

#undef int
