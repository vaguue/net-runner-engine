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

#undef int
