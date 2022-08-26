#include "wrapper.h"

void LogPacket(Ptr<PcapFileWrapper> file, Ptr<const Packet> p) {
  file->Write(Simulator::Now (), p);
}

Napi::Value Wrapper::initTracing(Napi::Env& env, const NodeCont& myNodes) {
  debug << "nodes count: " << myNodes.size() << endl;
  PcapHelper pcapHelper;
  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(fns::path{this->pcapPath} / fns::path{"main.pcap"}, std::ios::out, PcapHelper::DLT_PPP);
  for (auto& keyVal : myNodes) {
    auto& e = keyVal.second;
    debug << "working with node " << e.id << ' ' << e.type << ' ' << e.devs.GetN() << endl;
    uint32_t i = 0;
    for (auto it = e.devs.Begin(); i < e.devs.GetN(); ++i) {
      CsmaHelper csma;
      PointToPointHelper pointToPoint;
      YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
      string devName = e.devNames[i];
      if (devName.find("p2p") != string::npos) {
        pointToPoint.EnablePcap ((fns::path{this->pcapPath} / fns::path{devName + ".pcap"}).string(), *it, true, true);
      }
      else if (devName.find("csma") != string::npos) {
        csma.EnablePcap ((fns::path{this->pcapPath} / fns::path{devName + ".pcap"}).string(), *it, true, true);
      }
      else if (devName.find("wifi-ap") != string::npos) {
        phy.EnablePcap ((fns::path{this->pcapPath} / fns::path{devName + ".pcap"}).string(), *it, true, true);
      }
      /*else if (devName.find("wifi") != string::npos) {
        phy.EnablePcap ((fns::path{this->pcapPath} / fns::path{devName + ".pcap"}).string(), *it, true, true);
      }*/
      /*Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(fns::path{this->pcapPath} / fns::path{devName + ".pcap"}, std::ios::out, PcapHelper::DLT_PPP);
      debug << "dev name: " << devName << endl;
      debug << "device: " << (*it) << ' ';
      debug << (*it)->TraceConnectWithoutContext("PhyTxBegin", MakeBoundCallback(&LogPacket, file)) << ' ';
      debug << (*it)->TraceConnectWithoutContext("PhyTxEnd", MakeBoundCallback(&LogPacket, file)) << ' ';
      debug << (*it)->TraceConnectWithoutContext("PhyRxBegin", MakeBoundCallback(&LogPacket, file)) << ' ';
      debug << (*it)->TraceConnectWithoutContext("PhyRxEnd", MakeBoundCallback(&LogPacket, file)) << ' ' << endl;*/
      ++it;
    }
  }
  string animFile = fns::path{this->pcapPath} / fns::path{"anime.xml"};
  animePointer = new AnimationInterface(animFile);
  animePointer->SetMobilityPollInterval (Seconds (1));
  int i = 0;
  for (auto& keyVal : myNodes) {
    auto& e = keyVal.second;
    debug << "setting for node " << e.node.Get(0) << ' ' << i << endl;
    animePointer->SetConstantPosition (e.node.Get(0), e.x, e.y);
    i += 1;
  }
  animePointer->EnablePacketMetadata (true);
  //Ipv4RoutingHelper routingTrace;
  AsciiTraceHelper ascii;
  for (auto& keyVal : myNodes) {
    auto& e = keyVal.second;
    if (e.type == "pc") {
      auto path = fns::path{this->pcapPath} / fns::path{to_string(e.id) + "-arp.txt"};
      Ipv4RoutingHelper::PrintNeighborCacheEvery(Seconds(0.01), e.node.Get(0), ascii.CreateFileStream(path));
      //routingTrace.PrintArpCacheEvery(0.1, e.node.Get(0), ascii.CreateFileStream(path));
    }
  }
  return env.Null();
}
