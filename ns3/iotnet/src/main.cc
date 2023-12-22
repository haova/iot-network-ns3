#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/energy-module.h"
#include "ns3/internet-module.h"
#include "ns3/jamming-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

#include "ns3/iotnet.h"
#include "ns3/iotnet-helper.h"
#include "ns3/iotnet-wifi.h"
#include "ns3/iotnet-server.h"

NS_LOG_COMPONENT_DEFINE("IoTNetworkSimulation");

using namespace ns3;

void ReceivePacket(Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while (packet = socket->RecvFrom(from))
  {
    if (packet->GetSize() > 0)
    {
      InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(from);
      NS_LOG_UNCOND("Received one packet! Socket: " << iaddr.GetIpv4() << " port: " << iaddr.GetPort() << " at time = " << Simulator::Now().GetSeconds());
    }
  }
}

static void GenerateTraffic(Ptr<Socket> socket, uint32_t pktSize, Ptr<Node> n, uint32_t pktCount, Time pktInterval)
{
  if (pktCount > 0)
  {
    socket->Send(Create<Packet>(pktSize));
    Simulator::Schedule(pktInterval, &GenerateTraffic, socket, pktSize, n,
                        pktCount - 1, pktInterval);
  }
  else
  {
    socket->Close();
  }
}

void NodeRss(Ptr<WirelessModuleUtility> utilitySend, double time)
{
  Time t = Simulator::Now();
  double rss = utilitySend->GetRss();
  NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s Node RSS = " << rss << "W");
  if (t.GetSeconds() < time)
  {
    Simulator::Schedule(Seconds(0.2), &NodeRss, utilitySend, time);
  }
}

void NodePdr(Ptr<WirelessModuleUtility> utilitySend, double time)
{
  Time t = Simulator::Now();
  double pdr = utilitySend->GetPdr();
  NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s Node PDR = " << pdr);

  if (t.GetSeconds() < time)
  {
    Simulator::Schedule(Seconds(0.2), &NodePdr, utilitySend, time);
  }
}

int main(int argc, char *argv[])
{
  NS_LOG_UNCOND("Setting something up");

  // settings
  int nodeCount = 3;         // 2 normal [0, 1], 1 jammer [2]
  double TimeSimulation = 2; // seconds
  double interval = 0.1;     // seconds

  Time interPacketInterval = Seconds(interval);

  // logger
  // LogComponentEnable("MacLow", LOG_ALL);
  // LogComponentEnable("NslWifiChannel", LOG_ALL);

  // command
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // global
  IoTNet::world = CreateObject<IoTNet>();

  // server
  IoTNetServer server("server", "10.1.1.0", "255.255.255.0", Vector(0.0, 0.0, 0.0));

  // wifi network
  IoTNetWifi wifiA("wifi-a", "10.1.2.0", "255.255.255.0");
  Ptr<IoTNetNode> s1 = wifiA.Create("sensor-1", Vector(10.0, 10.0, 0.0));
  Ptr<IoTNetNode> s2 = wifiA.Create("sensor-2", Vector(20.0, 15.0, 0.0));
  Ptr<IoTNetNode> j = wifiA.Create("jammer", Vector(35.0, 32.0, 0.0));

  // old integrate
  NodeContainer c, networkNodes;
  c.Add(s1->node);
  c.Add(s2->node);
  c.Add(j->node);
  networkNodes.Add(s1->node);
  networkNodes.Add(s2->node);

  NetDeviceContainer devices, jammerNetdevice;
  devices.Add(s1->device);
  devices.Add(s2->device);
  jammerNetdevice.Add(j->device);

  // wireless utility
  WirelessModuleUtilityHelper utilityHelper;
  std::vector<std::string> AllInclusionList;
  std::vector<std::string> AllExclusionList;
  AllInclusionList.push_back("ns3::UdpHeader");          // record only UdpHeader
  AllExclusionList.push_back("ns3::olsr::PacketHeader"); // ignore all olsr headers/trailers
  utilityHelper.SetInclusionList(AllInclusionList);
  utilityHelper.SetExclusionList(AllExclusionList);
  WirelessModuleUtilityContainer utilities = utilityHelper.InstallAll();

  Ptr<WirelessModuleUtility> utilitySend = utilities.Get(0);    // node 0
  Ptr<WirelessModuleUtility> utilityReceive = utilities.Get(1); // node 1

  // jammer
  JammerHelper jammerHelper;
  jammerHelper.SetJammerType("ns3::ReactiveJammer");
  JammerContainer jammers = jammerHelper.Install(c.Get(nodeCount - 1));
  Ptr<Jammer> jammerPtr = jammers.Get(0);

  // jamming mitigation
  JammingMitigationHelper mitigationHelper;
  mitigationHelper.SetJammingMitigationType("ns3::MitigateByChannelHop");
  mitigationHelper.Set("MitigateByChannelHopChannelHopDelay",
                       TimeValue(Seconds(0.0)));
  mitigationHelper.Set("MitigateByChannelHopDetectionMethod",
                       UintegerValue(MitigateByChannelHop::PDR_ONLY));
  JammingMitigationContainer mitigators = mitigationHelper.Install(networkNodes); // normal
  Ptr<JammingMitigation> mitigationPtr = mitigators.Get(1);                       // node 1
  Ptr<JammingMitigation> mitigationPtr2 = mitigators.Get(0);                      // node 0

  // Install
  IoTNet::world->Install();

  AnimationInterface anim("output/iotnet-anim.xml");
  anim.EnablePacketMetadata();
  // IoTNet::world->UpdateAnimationInterface(anim);

  // schedule
  Simulator::Schedule(Seconds(1.3), &ns3::Jammer::StartJammer, jammerPtr); // start jammer at 7s

  // Simulator::Schedule(Seconds(0.1), NodePdr, utilitySend, TimeSimulation);
  // Simulator::Schedule(Seconds(0.1), NodePdr, utilityReceive, TimeSimulation);

  // Simulator::Schedule(Seconds(0.1), NodeRss, utilitySend, TimeSimulation);
  // Simulator::Schedule(Seconds(0.1), NodeRss, utilityReceive, TimeSimulation);

  // Simulator::Schedule(Seconds(startTime), &GenerateTraffic, source,
  //                     PpacketSize, networkNodes.Get(0), numPackets,
  //                     interPacketInterval);

  // simulation
  NS_LOG_UNCOND(">> Start simulation");
  Simulator::Stop(Seconds(TimeSimulation));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_UNCOND("<< Stop simulation");

  // information
  NS_LOG_UNCOND("PDR = " << utilityReceive->GetPdr());
  NS_LOG_UNCOND("Receiver = " << utilityReceive->GetValidPkts() << "/" << utilityReceive->GetTotalPkts());
  NS_LOG_UNCOND("Total Byte Received = " << utilityReceive->GetTotalBytesRx());
  NS_LOG_UNCOND("Total Byte Sent = " << utilitySend->GetTotalBytesTx());

  return 0;
}