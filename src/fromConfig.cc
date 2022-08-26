#include "wrapper.h"

/* examples config TODO require all ids to be 0 ... N ?
 * { nodes: [{ id, title, x, y, type, applications }], edges: [{ source, target, type, sourceIP, targetIP }] }
 */

/* convert to
 * map<int *id*, MyNode> and vector<vector<int>> *graph* Options
 */

#define int uint32_t

//const int CustomDataRate = 100000;
const string CustomDataRate = "5Mbps";
const int CustomDelay = 5;

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
    auto id = nodesArray.Get(i).As<Napi::Object>().Get("id").As<Napi::Number>().Uint32Value();
    auto x = nodesArray.Get(i).As<Napi::Object>().Get("x").As<Napi::Number>().DoubleValue();
    auto y = nodesArray.Get(i).As<Napi::Object>().Get("y").As<Napi::Number>().DoubleValue();
    auto type = string(nodesArray.Get(i).As<Napi::Object>().Get("type").As<Napi::String>());
    MyNode n(id, x, y, type);
    if (type == "wifi") {
      auto ssid = string(nodesArray.Get(i).As<Napi::Object>().Get("ssid").As<Napi::String>());
      auto apIP = string(nodesArray.Get(i).As<Napi::Object>().Get("apIP").As<Napi::String>());
      n.ssid = ssid;
      n.apIP = apIP;
    }
    if (nodesArray.Get(i).As<Napi::Object>().Has("applications")) {
      auto apps = nodesArray.Get(i).As<Napi::Object>().Get("applications").As<Napi::Array>();
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
    addr[source][target] = obj.Get("sourceIP").As<Napi::String>().Utf8Value();
    addr[target][source] = obj.Get("targetIP").As<Napi::String>().Utf8Value();
    debug << source << ' ' << target << ' ' << addr[source][target] << endl;
    debug << target << ' ' << source << ' ' << addr[target][source] << endl;
  }
  return addr;
}

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
  return res;
}
/* *** */

/* setting up NS-3 */

void CustomAssign (Ptr<NetDevice> device, string addr) {
  debug << "[*] assigning " << addr << endl;
  Ptr<Node> node = device->GetNode ();
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

  int32_t interface = ipv4->GetInterfaceForDevice (device);
  if (interface == -1)
  {
    interface = ipv4->AddInterface(device);
  }

  Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress(Ipv4Address(addr.c_str()), Ipv4Mask(uint32_t(0)));
  ipv4->AddAddress (interface, ipv4Addr);
  ipv4->SetMetric (interface, 1);
  ipv4->SetUp (interface);

  // Install the default traffic control configuration if the traffic
  // control layer has been aggregated, if this is not 
  // a loopback interface, and there is no queue disc installed already
  Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer> ();
  if (tc && DynamicCast<LoopbackNetDevice> (device) == 0 && tc->GetRootQueueDiscOnDevice (device) == 0)
  {
    Ptr<NetDeviceQueueInterface> ndqi = device->GetObject<NetDeviceQueueInterface> ();
    // It is useless to install a queue disc if the device has no
    // NetDeviceQueueInterface attached: the device queue is never
    // stopped and every packet enqueued in the queue disc is
    // immediately dequeued, hence there will never be backlog
    if (ndqi)
    {
      std::size_t nTxQueues = ndqi->GetNTxQueues ();
      TrafficControlHelper tcHelper = TrafficControlHelper::Default (nTxQueues);
      tcHelper.Install (device);
    }
  }
}

void setupNodes(NodeCont& myNodes) {
  for (auto& keyVal : myNodes) {//create each node
    auto& e = keyVal.second;
    NodeContainer tmp;
    tmp.Create(1);
    //e.node = tmp.Get(0);
    e.node = tmp;
  }
}

void addDevName(MyNode& node, const MyNode& other, string name) {
  int i = 0;
  for (auto e : node.devNames) {
    if (e == name) {
      i += 1;
    }
  }
  stringstream ss;
  ss << node.id << '-' << other.id << '-' << name << '-' << i;
  node.devNames.push_back(ss.str());
}

inline vector<pair<int, int>> getChildren(NodeCont& myNodes, const GraphCont& graph, const AddrCont& addrInfo, int root, vector<bool>& hubWas) {
  queue<int> q;
  q.push(root);
  hubWas[root] = true;
  vector<bool> used(myNodes.size());
  used[root] = true;
  decltype(getChildren(myNodes, graph, addrInfo, root, hubWas)) res;
  while (!q.empty()) {
    int i = q.front();
    q.pop();
    for (auto v : graph[i]) {
      if (!used[v]) {
        used[v] = true;
        if (myNodes[v].type == "hub") {
          hubWas[v] = true;
          q.push(v);
        }
        else {
          debug << "BFS: adding " << v << ' ' << i << ' ' << addrInfo[v][i] << endl;
          res.push_back(make_pair(v, i));
        }
      }
    }
  }
  return res;
}

void setupConnections(NodeCont& myNodes, const GraphCont& graph, const AddrCont& addrInfo) {
  for (auto& keyVal : myNodes) {
    auto& e = keyVal.second;
    if (e.type == "pc" || e.type == "wifi") {
      InternetStackHelper internet;
      internet.Install(e.node);
    }
  }
  vector<vector<bool>> was(myNodes.size(), vector<bool>(myNodes.size()));
  vector<bool> hubWas(myNodes.size());
  for (auto& keyVal : myNodes) {
    auto i = keyVal.first;
    auto& e = keyVal.second;
    if (e.type == "hub") { //TODO doesnt work
      if (hubWas[i]) continue;
      //debug << "initializating hub network from " << i << endl;
      CsmaHelper csma; //TODO config options
      //csma.SetChannelAttribute ("DataRate", DataRateValue (CustomDataRate));
      csma.SetChannelAttribute ("DataRate", StringValue (CustomDataRate));
      csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (CustomDelay)));
      NodeContainer hubNetwork;
      auto hubChildren = getChildren(myNodes, graph, addrInfo, i, hubWas);
      for (auto& chPr : hubChildren) {
        auto v = chPr.first;
        hubNetwork.Add(myNodes[v].node);
      }
      NetDeviceContainer link = csma.Install(hubNetwork);
      int j = 0;
      for (auto chPr : hubChildren) {
        auto v = chPr.first;
        auto& child = myNodes[v];
        child.devs.Add(link.Get(j));
        addDevName(child, e, "hub-csma");
        if (child.type == "pc") {
          debug << "assigning from hub " << addrInfo[v][chPr.second] << endl;
          CustomAssign(child.devs.Get(child.devs.GetN() - 1), addrInfo[v][chPr.second]);
        }
        was[chPr.second][v] = true;
        was[v][chPr.second] = true;
        j++;
      }
    }
  }
  for (auto& keyVal : myNodes) {
    auto i = keyVal.first;
    auto& e = keyVal.second;
    if (e.type == "switch") {
      CsmaHelper csma; //TODO config options
      //csma.SetChannelAttribute ("DataRate", DataRateValue (CustomDataRate));
      csma.SetChannelAttribute ("DataRate", StringValue (CustomDataRate));
      csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (CustomDelay)));
      for (auto v : graph[i]) {
        auto& child = myNodes[v];
        if (was[i][v]) continue;
        NetDeviceContainer link = csma.Install (NodeContainer(child.node, e.node));

        child.devs.Add(link.Get(0));
        e.devs.Add(link.Get(1));

        addDevName(child, e, "switch-csma");
        addDevName(e, child, "switch-csma");
        //child.devNames.push_back("switch-csma-" + child.id);//for pcaps
        //e.devNames.push_back("switch-csma-" + e.id);

        if (child.type == "pc") {
          debug << "assigning from switch " << v << endl;
          CustomAssign(child.devs.Get(child.devs.GetN() - 1), addrInfo[v][i]);
        }
        was[i][v] = true;
        was[v][i] = true;
      }
      OpenFlowSwitchHelper swtch;
      bool useDrop = false; //TODO config options;
      Time timeout = Seconds (0);
      debug << "switch dev numer: " << e.devs.GetN() << endl;
      if (useDrop)
      {
        Ptr<ofi::DropController> controller = CreateObject<ofi::DropController> ();
        swtch.Install (e.node.Get(0), e.devs, controller);
      }
      else
      {
        Ptr<ofi::LearningController> controller = CreateObject<ofi::LearningController> ();
        if (!timeout.IsZero ()) controller->SetAttribute ("ExpirationTime", TimeValue (timeout));
        swtch.Install (e.node.Get(0), e.devs, controller);
      }
      //TODO maybe csma.EnablePcapAll ("openflow-switch", false);???
    }
    else if (e.type == "wifi") {
      NodeContainer wifiStaNodes;
      for (auto v : graph[i]) {
        auto& child = myNodes[v];
        //if (was[i][v]) continue;
        wifiStaNodes.Add(child.node);
      }
      YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
      YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
      phy.SetChannel(channel.Create());
      WifiHelper wifi;
      //wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
      WifiMacHelper mac;
      Ssid ssid = Ssid(e.ssid);
      //TODO unknown config
      mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false)); 
      NetDeviceContainer staDevices = wifi.Install (phy, mac, wifiStaNodes);
      mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
      NetDeviceContainer apDevices = wifi.Install (phy, mac, e.node);
      e.devs.Add(apDevices);
      e.devNames.push_back("wifi-ap");
      int j = 0;
      for (auto v : graph[i]) {
        auto& child = myNodes[v];
        //if (was[i][v]) continue;
        child.devs.Add(staDevices.Get(j));
        addDevName(child, e, "wifi");
        CustomAssign(child.devs.Get(child.devs.GetN() - 1), addrInfo[v][i] != e.apIP && addrInfo[v][i].size() > 0 ? addrInfo[v][i] : addrInfo[i][v]);
        j++;
      }
      CustomAssign(e.devs.Get(e.devs.GetN() - 1), e.apIP);
    }
  }
  for (int i = 0; i < graph.size(); ++i) {
    for (int k = 0; k < graph[i].size(); ++k) {
      int j = graph[i][k];
      if (myNodes[i].type == "pc" && myNodes[j].type == "pc" && !was[i][j] && !was[j][i]) {
        PointToPointHelper pointToPoint;
        //pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (CustomDataRate)); //TODO config options
        pointToPoint.SetDeviceAttribute ("DataRate", StringValue (CustomDataRate)); //TODO config options
        pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (CustomDelay)));

        NetDeviceContainer p2pDevices;
        p2pDevices = pointToPoint.Install (myNodes[i].node.Get(0), myNodes[j].node.Get(0));

        myNodes[i].devs.Add(p2pDevices.Get(0));
        myNodes[j].devs.Add(p2pDevices.Get(1));

        addDevName(myNodes[i], myNodes[j], "p2p");
        addDevName(myNodes[j], myNodes[i], "p2p");
        //myNodes[i].devNames.push_back("pc-p2p-" + to_string(myNodes[i].id));
        //myNodes[j].devNames.push_back("pc-p2p-" + to_string(myNodes[j].id));

        CustomAssign(myNodes[i].devs.Get(myNodes[i].devs.GetN() - 1), addrInfo[i][j]);
        CustomAssign(myNodes[j].devs.Get(myNodes[j].devs.GetN() - 1), addrInfo[j][i]);

        was[i][j] = true;
        was[j][i] = true;
      }
    }
  }
}

pair<string, int> ipPort(string s) {
  string ip = "";
  int port = 0;
  int i = 0;
  for (; i < s.size(); ++i) {
    if (s[i] == ':') {
      ++i;
      break;
    }
    else {
      ip += s[i];
    }
  }
  istringstream(s.substr(i)) >> port;
  return make_pair(ip, port);
}

void setupApplications(NodeCont& myNodes, const options& opts) {
  for (auto keyVal : myNodes) {
    auto v = keyVal.second;
    for (auto e : v.apps) {
     ApplicationContainer apps;
      if (e.type == "ping") {
        V4PingHelper ping = V4PingHelper(Ipv4Address(e.dst.c_str()));
        ping.SetAttribute ("Interval", TimeValue (Seconds (e.interval)));
        apps = ping.Install(v.node.Get(0));
      }
      else if (e.type == "sink") {
        //InetSocketAddress dst = InetSocketAddress(Ipv4Address(e.dst.c_str())); //TODO maybe e.src ?
        InetSocketAddress dst = InetSocketAddress(Ipv4Address::GetAny()); //TODO maybe e.src ?
        PacketSinkHelper sink = PacketSinkHelper ("ns3::Ipv4RawSocketFactory", dst);
        apps = sink.Install (v.node.Get(0));
      }
      else if (e.type == "udp-server") {
        int port;
        istringstream(e.dst) >> port;
        UdpEchoServerHelper echoServer (port);
        apps = echoServer.Install (v.node.Get(0));
      }
      else if (e.type == "udp-client") {
        auto ipp = ipPort(e.dst);
        auto addr = ipp.first;
        auto port = ipp.second;
        UdpEchoClientHelper echoClient (Ipv4Address(addr.c_str()), port);
        echoClient.SetAttribute ("MaxPackets", UintegerValue (e.maxPackets));
        echoClient.SetAttribute ("Interval", TimeValue (Seconds (e.interval)));
        echoClient.SetAttribute ("PacketSize", UintegerValue (e.packetSize));
        apps = echoClient.Install (v.node.Get(0));
      }
      else if (e.type == "tcp-server") {
        int port;
        istringstream(e.dst) >> port;
        Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
        PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
        apps = sinkHelper.Install (v.node.Get(0));
      }
      else if (e.type == "tcp-client") {
        auto ipp = ipPort(e.dst);
        auto addr = ipp.first;
        auto port = ipp.second;
        BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address (addr.c_str()), port));
        // Set the amount of data to send in bytes.  Zero is unlimited.
        source.SetAttribute ("MaxBytes", UintegerValue (e.maxBytes));
        apps = source.Install (v.node.Get(0));
      }
      apps.Start (Seconds (e.start));
      if (e.stop > 0) {
        apps.Stop (Seconds (e.stop));
      }
      //apps.Stop (Seconds (e.stop > 0 ? e.stop : opts.animeLen));
    }
  }
}

Napi::Value Wrapper::fromConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Specify config").ThrowAsJavaScriptException();
  }
  auto config = info[0].As<Napi::Object>();

  auto myNodes = getNodes(config);
  auto graph = getGraph(config);
  auto options = getOpitons(config);
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
