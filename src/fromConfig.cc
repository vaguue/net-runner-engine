#include "wrapper.h"

using namespace Napi;
using namespace ns3;
using namespace std;

#define int uint32_t

DebugStream debug {true};

/* setting up NS-3 */
void setupNodes(NodeCont& myNodes) {
  for (auto& keyVal : myNodes) {//create each node
    auto& e = keyVal.second;
    NodeContainer tmp;
    tmp.Create(1);
    //e.node = tmp.Get(0);
    e.node = tmp;
  }
}

Napi::Value Wrapper::fromConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Specify config").ThrowAsJavaScriptException();
  }
  auto config = info[0].As<Napi::Object>();

  auto options = getOpitons(config);
  debug.enabled = options.verbose;

  auto myNodes = getNodes(config);
  auto graph = getGraph(config);
  auto addrInfo = getAddrInfo(config);

  debug << "[*] graph:" << endl;
  for (int i = 0; i < graph.size(); ++i) {
    for (auto v : graph[i]) {
      debug << i << ' ' << v << endl;
    }
  }

  setupNodes(myNodes);
  debug << "[*] setted up nodes" << endl;
  setupConnections(myNodes, graph, addrInfo);
  debug << "[*] setted up connections" << endl;
  setupApplications(myNodes, options);
  debug << "[*] setted up applications" << endl;

  debug << "[*] setted up" << endl;
  
  if (options.populateIP) {
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  }
  initTracing(env, myNodes);
  this->stopTime = options.animeLen;

  return env.Null();
}
/* *** */

#undef int
