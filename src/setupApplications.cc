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

Napi::String addrToJs(const Napi::CallbackInfo& info, Address addr) {
  Napi::Env env = info.Env();
  stringstream addrSS;
  addrSS << InetSocketAddress::ConvertFrom(addr).GetIpv4()
         << ':'
         << InetSocketAddress::ConvertFrom(addr).GetPort();
  return Napi::String::New(env, addrSS.str());
}

Napi::Buffer<char> packetToJs(const Napi::CallbackInfo& info, Ptr<const Packet> pkt) {
  Napi::Env env = info.Env();
  stringstream packetSS;
  pkt->CopyData(&packetSS, pkt->GetSize());
  string content = packetSS.str();
  return Napi::Buffer<char>::Copy(env, content.c_str(), content.size());
}

struct JsSink : public Application {
  JsSink();
  static TypeId GetTypeId(void);
  virtual ~JsSink ();
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  void setup(Ptr<Socket> socket, Address address, Callback<void, Ptr<Socket>> handleRead);

  void HandleAccept(Ptr<Socket> socket, const Address& from);
  void HandlePeerClose(Ptr<Socket> socket);
  void HandlePeerError(Ptr<Socket> socket);

  Callback<void, Ptr<Socket>> handleRead;
  list<Ptr<Socket>> socketList;
  Ptr<Socket> socket;
  Address peer;
  bool running;
};

void JsSink::HandleAccept(Ptr<Socket> s, const Address& from) {
  s->SetRecvCallback(handleRead);
  socketList.push_back(s);
}
void JsSink::HandlePeerClose(Ptr<Socket> socket) {
}

void JsSink::HandlePeerError(Ptr<Socket> socket) {

}


JsSink::JsSink() : socket(0), peer(), running(false) {}
JsSink::~JsSink() { socket = 0; }
TypeId JsSink::GetTypeId (void) {
  static TypeId tid = TypeId("JsSink")
    .SetParent<Application>()
    .SetGroupName("Net-runner-engine")
    .AddConstructor<JsSink>()
    ;
  return tid;
}
void JsSink::setup(Ptr<Socket> socket, Address address, Callback<void, Ptr<Socket>> handleRead) {
  this->handleRead = handleRead;
  this->socket = socket;
  this->peer = address;
}

void JsSink::StartApplication (void) {
  running = true;
  socket->Bind(peer);
  socket->Listen();
  socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address &>(), MakeCallback(&JsSink::HandleAccept, this));
  socket->SetRecvPktInfo(true);
  socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address &> (), MakeCallback (&JsSink::HandleAccept, this));
  socket->SetCloseCallbacks(MakeCallback (&JsSink::HandlePeerClose, this), MakeCallback (&JsSink::HandlePeerError, this));
}
void JsSink::StopApplication (void) {
  running = false;
  while(!socketList.empty()) {
    Ptr<Socket> acceptedSocket = socketList.front ();
    socketList.pop_front();
    acceptedSocket->Close();
  }
  if (socket) {
    socket->Close();
    socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
  }
}

struct JsApp : public Application {
  JsApp();
  static TypeId GetTypeId(void);
  virtual ~JsApp ();
  virtual void StartApplication(void);
  virtual void StopApplication(void);
  void tick(string delay = "");
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
void JsApp::tick(string customDelay) {
  string resDelay = interval.size() > 0 ? interval : (customDelay.size() > 0 ? customDelay : "0ms");
  if (running) {
    sendEvent = Simulator::Schedule(Time(resDelay), &JsApp::callOnTick, this);
  }
}
void JsApp::callOnTick() {
  Napi::Env env = info->Env();
  Napi::Object arg = Napi::Object::New(env);
  arg.Set("time", Simulator::Now().GetMilliSeconds());
  arg.Set("sendPacket", premadeCallback);
  auto res = onTick.Call({arg});
  if (res.IsString()) {
    tick(res.As<Napi::String>().Utf8Value());
  }
  /*else if (res.IsPromise()) {

  }*/
  else if (interval.size() > 0) {
    tick();
  }
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

void sinkRxTrace(const Napi::CallbackInfo* info, Napi::Function onRecieve, Ptr<Socket> socket) {
  Napi::Env env = info->Env();
  Ptr<Packet> packet;
  Address from;
  Address localAddress;

  while ((packet = socket->RecvFrom(from))) {
    socket->GetSockName(localAddress);
    function<void(const Napi::CallbackInfo&)> reply = [&](const Napi::CallbackInfo& info) {
      if (info.Length() > 0) {
        auto buf = info[0].As<Napi::Buffer<uint8_t>>();
        Ptr<Packet> pkt = Create<Packet>(reinterpret_cast<const uint8_t*>(buf.Data()), buf.Length());
        socket->SendTo(pkt, 0, from);
      }
    };

    Napi::Object arg = Napi::Object::New(env);

    arg.Set("address", addrToJs(*info, from));
    arg.Set("packet", packetToJs(*info, packet));
    arg.Set("reply", Napi::Function::New(env, reply));

    onRecieve.Call({arg});
  }
};

struct UdpServerInfo {
  const Napi::CallbackInfo& info;
  string addr = "";
  int port;

  bool echo = false;
  bool traceRx = false;
  Napi::Function onRecieve;
  UdpServerInfo(const Napi::Object& init, const Napi::CallbackInfo& info) : info{info} {
    if (init.Get("dst").IsNumber()) {
      port = init.Get("dst").As<Napi::Number>().Uint32Value();
    }
    else {
      string dst = init.Get("dst").As<Napi::String>();
      auto ipp = ipPort(dst);
      addr = ipp.first;
      port = ipp.second;
    }
    if (init.Has("onRecieve") && init.Get("onRecieve").IsFunction()) {
      traceRx = true;
      onRecieve = init.Get("onRecieve").As<Napi::Function>();
    }
    echo = init.Has("echo");
  };
  void install(ApplicationContainer& apps, MyNode& v) {
    auto resIp = addr.size() > 0 ? Ipv4Address(addr.c_str()) : Ipv4Address::GetAny();
    if (traceRx) {
      auto handleRead = MakeBoundCallback(&sinkRxTrace, &info, onRecieve);
      Ptr<Socket> ns3UdpSocket = Socket::CreateSocket(v.node.Get(0), UdpSocketFactory::GetTypeId());
      Ptr<JsSink> app = CreateObject<JsSink>();
      app->setup(ns3UdpSocket, Address(InetSocketAddress(resIp, port)), handleRead);
      v.node.Get(0)->AddApplication(app);
      apps.Add(app);
      ns3UdpSocket->SetRecvCallback(handleRead);
    }
    else if (echo) {
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
  const Napi::CallbackInfo& info;
  string addr = "";
  int port;

  bool traceRx = false;
  Napi::Function onRecieve;
  TcpServerInfo(const Napi::Object& init, const Napi::CallbackInfo& info) : info{info} {
    if (init.Get("dst").IsNumber()) {
      port = init.Get("dst").As<Napi::Number>().Uint32Value();
    }
    else {
      string dst = init.Get("dst").As<Napi::String>();
      auto ipp = ipPort(dst);
      addr = ipp.first;
      port = ipp.second;
    }
    if (init.Has("onRecieve") && init.Get("onRecieve").IsFunction()) {
      traceRx = true;
      onRecieve = init.Get("onRecieve").As<Napi::Function>();
    }
  }
  void install(ApplicationContainer& apps, MyNode& v) {
    auto resIp = addr.size() > 0 ? Ipv4Address(addr.c_str()) : Ipv4Address::GetAny();
    if (traceRx) {
      auto handleRead = MakeBoundCallback(&sinkRxTrace, &info, onRecieve);
      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(v.node.Get(0), TcpSocketFactory::GetTypeId());
      Ptr<JsSink> app = CreateObject<JsSink>();
      app->setup(ns3TcpSocket, Address(InetSocketAddress(resIp, port)), handleRead);
      v.node.Get(0)->AddApplication(app);
      apps.Add(app);
      ns3TcpSocket->SetRecvCallback(handleRead);
    }
    else {
      Address sinkLocalAddress (InetSocketAddress (resIp, port));
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
      apps = sinkHelper.Install (v.node.Get(0));
    }
  }
};

struct UdpClientInfo {
  const Napi::CallbackInfo& info;
  string addr;
  int port;

  int maxBytes = 0;
  //int maxPackets = 20;
  int packetSize = 256;
  string dataRate = "5Mbps";
  bool bulk = false;

  Napi::Function onTick;
  string tickInterval = "";
  bool useTicks = false;

  UdpClientInfo(const Napi::Object& init, const Napi::CallbackInfo& info) : info{info} {
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

    if (init.Has("onTick") && init.Get("onTick").IsFunction()) {
      useTicks = true;
      onTick = init.Get("onTick").As<Napi::Function>();
    }
    if (init.Has("tickInterval") && init.Get("tickInterval").IsString()) {
      tickInterval = init.Get("tickInterval").As<Napi::String>();
    }
  }
  void install(ApplicationContainer& apps, MyNode& v) {
    if (useTicks) {
      Ptr<Socket> ns3UdpSocket = Socket::CreateSocket(v.node.Get(0), UdpSocketFactory::GetTypeId());
      Ptr<JsApp> app = CreateObject<JsApp>();
      app->setup(&info, ns3UdpSocket, Address(InetSocketAddress(Ipv4Address(addr.c_str()), port)), onTick, tickInterval);
      v.node.Get(0)->AddApplication(app);
      apps.Add(app);
    }
    //TODO bulk for UDP is not supported by NS3
    /*else if (bulk) {
      BulkSendHelper source("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address (addr.c_str()), port));
      source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
      apps = source.Install(v.node.Get(0));
    }*/ 
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

    bulk = init.Has("bulk");

    if (init.Has("onTick") && init.Get("onTick").IsFunction()) {
      useTicks = true;
      onTick = init.Get("onTick").As<Napi::Function>();
    }
    if (init.Has("tickInterval") && init.Get("tickInterval").IsString()) {
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

struct RawSocketClientInfo {
  const Napi::CallbackInfo& info;
  string addr = "";
  int port;

  bool packetSocket = false;

  Napi::Function onTick;
  string tickInterval = "";
  bool useTicks = false;

  RawSocketClientInfo(const Napi::Object& init, const Napi::CallbackInfo& info) : info{info} {
    string dst = init.Get("dst").As<Napi::String>();
    auto ipp = ipPort(dst);
    addr = ipp.first;
    port = ipp.second;
    if (init.Has("onTick") && init.Get("onTick").IsFunction()) {
      useTicks = true;
      onTick = init.Get("onTick").As<Napi::Function>();
    }
    if (init.Has("tickInterval") && init.Get("tickInterval").IsString()) {
      tickInterval = init.Get("tickInterval").As<Napi::String>();
    }
    if (init.Has("packetSocket")) {
      packetSocket = init.Get("packetSocket").As<Napi::Boolean>();
    }
  }
  void install(ApplicationContainer& apps, MyNode& v) {
    if (useTicks) {
      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(v.node.Get(0), 
          packetSocket ? 
          PacketSocketFactory::GetTypeId() :
          Ipv4RawSocketFactory::GetTypeId());
      Ptr<JsApp> app = CreateObject<JsApp>();
      app->setup(&info, ns3TcpSocket, Address(InetSocketAddress(Ipv4Address(addr.c_str()), port)), onTick, tickInterval);
      v.node.Get(0)->AddApplication(app);
      apps.Add(app);
    } 
    else {
      throw Napi::Error::New(info.Env(), "Raw socket application has to have onTick/onRecieve argument");
    }
  }
};

struct RawSocketServerInfo {
  const Napi::CallbackInfo& info;
  string addr = "";
  int port;

  bool packetSocket = false;
  bool traceRx = false;
  Napi::Function onRecieve;
  RawSocketServerInfo(const Napi::Object& init, const Napi::CallbackInfo& info) : info{info} {
    if (init.Get("dst").IsNumber()) {
      port = init.Get("dst").As<Napi::Number>().Uint32Value();
    }
    else {
      string dst = init.Get("dst").As<Napi::String>();
      auto ipp = ipPort(dst);
      addr = ipp.first;
      port = ipp.second;
    }
    if (init.Has("onRecieve") && init.Get("onRecieve").IsFunction()) {
      traceRx = true;
      onRecieve = init.Get("onRecieve").As<Napi::Function>();
    }
    if (init.Has("packetSocket")) {
      packetSocket = init.Get("packetSocket").As<Napi::Boolean>();
    }
  }
  void install(ApplicationContainer& apps, MyNode& v) {
    auto resIp = addr.size() > 0 ? Ipv4Address(addr.c_str()) : Ipv4Address::GetAny();
    if (traceRx) {
      auto handleRead = MakeBoundCallback(&sinkRxTrace, &info, onRecieve);
      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(v.node.Get(0), 
          packetSocket ? 
          PacketSocketFactory::GetTypeId() :
          Ipv4RawSocketFactory::GetTypeId());
      Ptr<JsSink> app = CreateObject<JsSink>();
      app->setup(ns3TcpSocket, Address(InetSocketAddress(resIp, port)), handleRead);
      v.node.Get(0)->AddApplication(app);
      apps.Add(app);
      ns3TcpSocket->SetRecvCallback(handleRead);
    }
    else {
      throw Napi::Error::New(info.Env(), "Raw socket application has to have onTick/onRecieve argument");
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
      else if (e.type == "raw-socket-client") {
        RawSocketClientInfo appInfo{e.init, info};
        appInfo.install(apps, v);
      }
      else if (e.type == "raw-socket-server") {
        RawSocketServerInfo appInfo{e.init, info};
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
