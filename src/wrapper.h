#pragma once

#include <napi.h>
#include <bits/stdc++.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/openflow-module.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/trace-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/ipv4-routing-helper.h"

namespace fns = std::filesystem;

struct DebugStream {
  struct NullStream : public std::ostream {
    struct NullBuffer : public std::streambuf {
      int overflow(int c) override { return c; }
    } buffer;
    NullStream() : std::ostream(&buffer) {}
  } null;

  std::ostream& output;
  bool enabled;
  DebugStream(bool enabled = false, std::ostream &output = std::cout) : output(output), enabled(enabled) {}
  void enable(const bool enable) { enabled = enable; }
  template <typename T> std::ostream& operator<<(const T &arg) {
      if (enabled) return output << arg;
      else return null << arg;
  }
};

DebugStream debug {true};

struct options {
  bool populateIP = true;
  double animeLen = 10;
};

struct application {
  std::string type;
  std::string src;
  std::string dst;
  std::string port;
  Napi::Object init;
//upd only
  int maxPackets = 20;
  int packetSize = 256;
  //double interval = 1.0;
  double interval = 0.5;
//
//tcp only
  int maxBytes = 0;
//
  double start = 0;
  double stop = -1;
};

struct MyNode {
  int id;
  double x;
  double y;
  std::string type;
  std::vector<application> apps;
  //Ptr<Node> node;
  ns3::NodeContainer node;
  ns3::NetDeviceContainer devs;
  std::vector<std::string> devNames;
//wifi only
  std::string ssid;
  std::string apIP;
//
  MyNode(int id, double x, double y, std::string type) : id{id}, x{x}, y{y}, type{type} {};
  MyNode() {};
};

using NodeCont = std::map<int, MyNode>;
using GraphCont = std::vector<std::vector<int>>;
using AddrCont = std::vector<std::vector<std::string>>;

struct  Wrapper : public Napi::ObjectWrap<Wrapper>
{
  Wrapper(const Napi::CallbackInfo&);
  void clear();
  ns3::NodeContainer getConfigNodes(const Napi::Object&);
  Napi::Value setLogger(const Napi::CallbackInfo& info);
  Napi::Value setPcapPath(const Napi::CallbackInfo& info);
  Napi::Value fromConfig(const Napi::CallbackInfo&);
  Napi::Value runFromConfig(const Napi::CallbackInfo&);
  Napi::Value initTracing(Napi::Env&, const NodeCont&);
  Napi::Value run(const Napi::CallbackInfo&);  
  static Napi::Function GetClass(Napi::Env);

  double stopTime;
  std::string pcapPath = "da.pcap";
  Napi::FunctionReference logger;
  std::string _greeterName;
  ns3::AnimationInterface* animePointer;
};
