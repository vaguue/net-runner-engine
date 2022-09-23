#include "wrapper.h"

using namespace Napi;
using namespace ns3;
using namespace std;

#define int uint32_t

extern struct DebugStream debug;

/* examples config TODO require all ids to be 0 ... N ?
 * { nodes: [{ id, title, x, y, type, applications }], edges: [{ source, target, type, sourceIP, targetIP }] }
 */

/* convert to
 * map<int *id*, MyNode> and vector<vector<int>> *graph* Options
 */

/* GET C++ Objects from Node.js */

application getApplication(const Napi::Object& obj) {
  application res;
  res.type = obj.Get("type").As<Napi::String>();
  if (obj.Has("dst")) {
    res.dst = obj.Get("dst").As<Napi::String>();
  }
  if (obj.Has("src")) {
    res.src = obj.Get("src").As<Napi::String>();
  }
  if (obj.Has("port")) {
    res.port = obj.Get("port").As<Napi::String>();
  }
  if (obj.Has("interval")) {
    res.interval = obj.Get("interval").As<Napi::Number>().DoubleValue();
  }
  if (obj.Has("packetSize")) {
    res.packetSize = obj.Get("packetSize").As<Napi::Number>().Uint32Value();
  }
  if (obj.Has("maxPackets")) {
    res.maxPackets = obj.Get("maxPackets").As<Napi::Number>().Uint32Value();
  }
  res.init = obj;
  return res;
}

NodeCont getNodes(const Napi::Object& config) {
  auto nodesArray = config.Get("nodes").As<Napi::Array>();
  NodeCont res;
  for (int i = 0; i < nodesArray.Length(); ++i) {
    auto cur = nodesArray.Get(i).As<Napi::Object>();
    auto id = cur.Get("id").As<Napi::Number>().Uint32Value();
    auto x = cur.Get("x").As<Napi::Number>().DoubleValue();
    auto y = cur.Get("y").As<Napi::Number>().DoubleValue();
    auto type = string(cur.Get("type").As<Napi::String>());
    string name = cur.Has("name") && cur.Get("name").IsString() ? string(cur.Get("name").As<Napi::String>()) : "";
    MyNode n(id, x, y, type);
    n.init = nodesArray.Get(i).As<Napi::Object>();
    n.name = name;
    if (type == "wifi") {
      auto ssid = string(cur.Get("ssid").As<Napi::String>());
      auto apIP = string(cur.Get("apIP").As<Napi::String>());
      n.ssid = ssid;
      n.apIP = apIP;
    }
    if (cur.Has("applications")) {
      auto apps = cur.Get("applications").As<Napi::Array>();
      for (int j = 0; j < apps.Length(); ++j) {
        n.apps.push_back(getApplication(apps.Get(j).As<Napi::Object>()));
      }
    }
    res[id] = n;
  }
  return res;
}

GraphCont getGraph(const Napi::Object& config) {
  auto edgesArray = config.Get("edges").As<Napi::Array>();
  auto nodesArray = config.Get("nodes").As<Napi::Array>();
  GraphCont graph(nodesArray.Length());
  for (int i = 0; i < edgesArray.Length(); ++i) {
    auto obj = edgesArray.Get(i).As<Napi::Object>();
    auto source = obj.Get("source").As<Napi::Number>().Uint32Value();
    auto target = obj.Get("target").As<Napi::Number>().Uint32Value();
    graph[source].push_back(target);
    graph[target].push_back(source);
  }
  return graph;
}

AddrCont getAddrInfo(const Napi::Object& config) {
  auto edgesArray = config.Get("edges").As<Napi::Array>();
  auto nodesArray = config.Get("nodes").As<Napi::Array>();
  AddrCont addr(nodesArray.Length(), vector<string>(nodesArray.Length()));
  for (int i = 0; i < edgesArray.Length(); ++i) {
    auto obj = edgesArray.Get(i).As<Napi::Object>();
    auto source = obj.Get("source").As<Napi::Number>().Uint32Value();
    auto target = obj.Get("target").As<Napi::Number>().Uint32Value();
    if (obj.Has("sourceIP")) {
      addr[source][target] = obj.Get("sourceIP").As<Napi::String>().Utf8Value();
    }
    if (obj.Has("targetIP")) {
      addr[target][source] = obj.Get("targetIP").As<Napi::String>().Utf8Value();
    }
    debug << "[DEBUG] " << source << ' ' << target << ' ' << addr[source][target] << endl;
    debug << "[DEBUG] " << target << ' ' << source << ' ' << addr[target][source] << endl;
  }
  return addr;
}

ConnectionDataCont getConnectionData(const Napi::Object& config, NodeCont& myNodes) {
  auto edgesArray = config.Get("edges").As<Napi::Array>();
  auto nodesArray = config.Get("nodes").As<Napi::Array>();
  ConnectionDataCont conn(nodesArray.Length(), vector<ConnectionData>(nodesArray.Length()));
  for (int i = 0; i < edgesArray.Length(); ++i) {
    auto obj = edgesArray.Get(i).As<Napi::Object>();
    auto source = obj.Get("source").As<Napi::Number>().Uint32Value();
    auto target = obj.Get("target").As<Napi::Number>().Uint32Value();
    auto sourceNode = myNodes[source];
    auto targetNode = myNodes[target];
    if (sourceNode.type != "hub" && targetNode.type != "hub") {
      ConnectionData tmp;
      if (obj.Has("dataRate")) {
        tmp.customDataRate = string(obj.Get("dataRate").As<Napi::String>());
      }
      if (obj.Has("delay")) {
        tmp.customDelay = string(obj.Get("delay").As<Napi::String>());
      }
      conn[source][target] = tmp;
      conn[target][source] = tmp;
    }
  }
  return conn;
};

options getOpitons(const Napi::Object& config) {
  options res;
  if (!config.Has("options")) {
    return res;
  }
  auto configOptions = config.Get("options").As<Napi::Object>();
  if (configOptions.Has("populateIP")) {
    res.populateIP = configOptions.Get("populateIP").As<Napi::Boolean>().Value();
  }
  if (configOptions.Has("animeLen")) {
    res.animeLen = configOptions.Get("animeLen").As<Napi::Number>().DoubleValue();
  }
  if (configOptions.Has("verbose")) {
    res.verbose = configOptions.Get("verbose").As<Napi::Boolean>();
  }
  return res;
}

#undef int
