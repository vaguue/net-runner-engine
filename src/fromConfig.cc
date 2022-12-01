#include "wrapper.h"

using namespace Napi;
using namespace ns3;
using namespace std;

#define int uint32_t

DebugStream debug {true};

/* setting up NS-3 */
void setupNodes(NodeCont& myNodes, const string& runId = "") {
  for (auto& keyVal : myNodes) {//create each node
    auto& e = keyVal.second;
    NodeContainer tmp;
    tmp.Create(1);
    //e.node = tmp.Get(0);
    e.node = tmp;
    if (e.name.size() > 0) {
      if (runId.size() > 0) {
        Names::Add(("/Names/"+runId+"@"+e.name).c_str(), e.node.Get(0));
      }
      else {
        Names::Add(("/Names/"+e.name).c_str(), e.node.Get(0));
      }
    }
  }
}

Napi::Value Wrapper::fromConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Specify config").ThrowAsJavaScriptException();
  }
  auto config = info[0].As<Napi::Object>();

  string runId = config.Has("runId") ? config.Get("runId").As<Napi::String>() : string("");

  auto options = getOpitons(config);
  debug.enabled = options.verbose;

  auto myNodes = getNodes(config);
  auto graph = getGraph(config);
  auto addrInfo = getAddrInfo(config);
  auto connectionData = getConnectionData(config, myNodes);

  DEBUG(connectionData);
  DEBUG(graph);

  setupNodes(myNodes, runId);
  debug << "[DEBUG] setted up nodes" << endl;
  setupConnections(myNodes, graph, addrInfo, connectionData);
  debug << "[DEBUG] setted up connections" << endl;
  setupApplications(myNodes, options, info);
  debug << "[DEBUG] setted up applications" << endl;

  debug << "[DEBUG] setted up" << endl;
  
  if (options.populateIP) {
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  }
  initTracing(env, myNodes);
  this->stopTime = options.animeLen;

  return env.Null();
}
/* *** */

#undef int
