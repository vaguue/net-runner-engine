#include "wrapper.h"
#include "ns3/mobility-helper.h"

using namespace Napi;
using namespace ns3;
using namespace std;

#define int uint32_t

extern struct DebugStream debug;

void LogPacket(Ptr<PcapFileWrapper> file, Ptr<const Packet> p) {
  file->Write(Simulator::Now(), p);
}

vector<string> Wrapper::initTracing(Napi::Env& env, const NodeCont& myNodes) {
  DEBUG(myNodes.size());
  vector<string> resFiles;
  PcapHelper pcapHelper;
  //Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(fns::path{this->pcapPath} / fns::path{"main.pcap"}, std::ios::out, PcapHelper::DLT_RAW);
  for (auto& keyVal : myNodes) {
    auto& e = keyVal.second;
    debug << "[DEBUG] working with node " << e.id << ' ' << e.type << ' ' << e.devs.GetN() << endl;
    uint32_t i = 0;
    for (auto it = e.devs.Begin(); i < e.devs.GetN(); ++i) {
      CsmaHelper csma;
      PointToPointHelper pointToPoint;
      YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
      string devName = e.devNames[i];
      string fpath = (fns::path{this->pcapPath} / fns::path{devName + ".pcap"}).string();
      if (devName.find("p2p") != string::npos) {
        pointToPoint.EnablePcap (fpath, *it, true, true);
      }
      else if (devName.find("csma") != string::npos) {
        csma.EnablePcap (fpath, *it, true, true);
      }
      else if (devName.find("wifi-ap") != string::npos) {
        phy.EnablePcap (fpath, *it, true, true);
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

      resFiles.push_back(fpath);
      ++it;
    }
  }
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  for (auto& keyVal : myNodes) {
    auto& e = keyVal.second;
    mobility.Install(e.node);
  }
  string animFile = fns::path{this->pcapPath} / fns::path{"anime.xml"};
  animePointer = new AnimationInterface(animFile);
  animePointer->SetMobilityPollInterval (Seconds (1));
  for (auto& keyVal : myNodes) {
    auto& e = keyVal.second;
    debug << "[DEBUG] setting for node " << e.node.Get(0)->GetId() << endl;
    animePointer->SetConstantPosition (e.node.Get(0), e.x, e.y);
  }
  resFiles.push_back(animFile);
  animePointer->EnablePacketMetadata (true);
  Ipv4GlobalRoutingHelper g;
  AsciiTraceHelper ascii;
  for (auto& keyVal : myNodes) {
    auto& e = keyVal.second;
    if (e.type == "pc") {
      auto arpPath = fns::path{this->pcapPath} / fns::path{e.getName() + "-arp.txt"};
      Ipv4RoutingHelper::PrintNeighborCacheEvery(Seconds(0.01), e.node.Get(0), ascii.CreateFileStream(arpPath));

      auto routingTablePath = fns::path{this->pcapPath} / fns::path{e.getName() + "-routing.routes"};
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(routingTablePath, std::ios::out);
      g.PrintRoutingTableAt(Seconds(this->stopTime), e.node.Get(0), routingStream);

      resFiles.push_back(arpPath);
      resFiles.push_back(routingTablePath);
    }
  }
  return resFiles;
}

#undef int
