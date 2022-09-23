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
  int port;
  UdpServerInfo(const Napi::Object& init) {
    if (init.Get("dst").IsNumber()) {
      port = init.Get("dst").As<Napi::Number>().Uint32Value();
    }
    else {
      string dst = init.Get("dst").As<Napi::String>();
      istringstream(dst) >> port;
    }
  };
  void install(ApplicationContainer& apps, MyNode& v) {
    UdpEchoServerHelper echoServer (port);
    apps = echoServer.Install (v.node.Get(0));
  }
};

struct UdpClientInfo {
  string addr;
  int port;
  int maxPackets = 20;
  int packetSize = 256;
  string interval = "0.5s";
  UdpClientInfo(const Napi::Object& init) {
    string dst = init.Get("dst").As<Napi::String>();
    auto ipp = ipPort(dst);
    addr = ipp.first;
    port = ipp.second;
    if (init.Has("maxPackets")) {
      maxPackets = init.Get("maxPackets").As<Napi::Number>().Uint32Value();
    }
    if (init.Has("packetSize")) {
      packetSize = init.Get("packetSize").As<Napi::Number>().Uint32Value();
    }
    if (init.Has("interval")) {
      interval = init.Get("interval").As<Napi::String>();
    }
  }
  void install(ApplicationContainer& apps, MyNode& v) {
    UdpEchoClientHelper echoClient (Ipv4Address(addr.c_str()), port);
    echoClient.SetAttribute ("MaxPackets", UintegerValue(maxPackets));
    echoClient.SetAttribute ("Interval", TimeValue(Time(interval)));
    echoClient.SetAttribute ("PacketSize", UintegerValue(packetSize));
    apps = echoClient.Install (v.node.Get(0));
  }
};

struct TcpServerInfo {
  string addr = "";
  int port;
  TcpServerInfo(const Napi::Object& init) {
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
    if (addr.size() > 0) {
      Address sinkLocalAddress (InetSocketAddress (Ipv4Address(addr.c_str()), port));
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
      apps = sinkHelper.Install (v.node.Get(0));
    }
    else {
      Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
      apps = sinkHelper.Install (v.node.Get(0));
    }
  }
};

struct TcpClientInfo {
  string addr = "";
  int port;
  int maxBytes = 0;
  TcpClientInfo(const Napi::Object& init) {
    string dst = init.Get("dst").As<Napi::String>();
    auto ipp = ipPort(dst);
    addr = ipp.first;
    port = ipp.second;
    if (init.Has("maxBytes")) {
      maxBytes = init.Get("maxBytes").As<Napi::Number>().Uint32Value();
    }
  }
  void install(ApplicationContainer& apps, MyNode& v) {
    BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address (addr.c_str()), port));
    source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    apps = source.Install (v.node.Get(0));
  }
};

void setupApplications(NodeCont& myNodes, const options& opts) {
  for (auto keyVal : myNodes) {
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
        UdpServerInfo appInfo{e.init};
        appInfo.install(apps, v);
      }
      else if (e.type == "udp-client") {
        UdpClientInfo appInfo{e.init};
        appInfo.install(apps, v);
      }
      else if (e.type == "tcp-server") {
        TcpServerInfo appInfo{e.init};
        appInfo.install(apps, v);
      }
      else if (e.type == "tcp-client") {
        TcpClientInfo appInfo{e.init};
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
