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
#include "ns3/bridge-helper.h"

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
  template <typename T> 
  std::ostream& operator<<(const T& arg) {
    if (enabled) return output << arg;
    else return null << arg;
  }
};

#define DEBUG(x) debug << "[DEBUG] " << #x << ": " << (x) << endl;

struct options {
  bool populateIP = false;
  bool verbose = false;
  double animeLen = 10;
};

struct application {
  std::string type;
  Napi::Object init;
  std::string start = "0s";
  std::string stop = "end";
};

struct MyNode {
  int id;
  double x;
  double y;
  std::string type;
  std::string name = "";
  std::vector<application> apps;
  //Ptr<Node> node;
  ns3::NodeContainer node;
  ns3::NetDeviceContainer devs;
  std::vector<std::string> devNames;
  Napi::Object init;
//wifi only
  std::string ssid;
  std::string apIP;
//
  MyNode(int id, double x, double y, std::string type) : id{id}, x{x}, y{y}, type{type} {};
  MyNode() {};
  std::string getName() const {
    if (name.size() > 0) return name;
    return std::to_string(id);
  };
};

using NodeCont = std::map<int, MyNode>;
using GraphCont = std::vector<std::vector<int>>;
using AddrCont = std::vector<std::vector<std::pair<std::string, std::string>>>;

struct ConnectionData {
  //int CustomDataRate = 100000;
  //int customDelay = 5;
  std::string customDataRate = "5Mbps";
  std::string customDelay = "5ms";
};

using ConnectionDataCont = std::vector<std::vector<ConnectionData>>;

struct Wrapper : public Napi::ObjectWrap<Wrapper> {
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

/*template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T>& p);
template<typename T, typename R>
std::ostream& operator<<(std::ostream& os, std::map<T, R>& p);
std::ostream& operator<<(std::ostream& os, ConnectionData& p);*/

template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T>& p) {
  os << '{';
  for (size_t i = 0; i < p.size(); ++i) {
    if (i) os << ' ';
    os << p[i];
  }
  os << '}';
  return os;
}

template<typename T, typename R>
std::ostream& operator<<(std::ostream& os, std::map<T, R>& p) {
  os << '{';
  bool first = true;
  for (auto e : p) {
    if (!first) os << ' ';
    first = false;
    os << e;
  }
  os << '}';
  return os;
}

template<typename T = ConnectionData>
std::ostream& operator<<(std::ostream& os, ConnectionData& p) {
  os << "ConnectionData{" << p.customDataRate << ' ' << p.customDelay << '}';
  return os;
}

#define toNumber(x) Napi::Number(env, (x))
#define asString(x) (x).As<Napi::String>().Utf8Value()

application getApplication(const Napi::Object& obj);
NodeCont getNodes(const Napi::Object& config);
GraphCont getGraph(const Napi::Object& config);
AddrCont getAddrInfo(const Napi::Object& config);
ConnectionDataCont getConnectionData(const Napi::Object& config, NodeCont& myNodes);
options getOpitons(const Napi::Object& config);
void setupApplications(NodeCont& myNodes, const options& opts, const Napi::CallbackInfo& info);
void setupConnections(NodeCont& myNodes, const GraphCont& graph, const AddrCont& addrInfo, const ConnectionDataCont& connectionData);
