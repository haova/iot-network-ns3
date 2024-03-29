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
#include "ns3/iotnet-router.h"

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
  int nodeCount = 3;     // 2 normal [0, 1], 1 jammer [2]
  double duration = 30;  // seconds
  double interval = 0.1; // seconds

  bool realtime = false;
  bool jamming = false;

  Time interPacketInterval = Seconds(interval);

  // logger
  // LogComponentEnable("MacLow", LOG_ALL);
  // LogComponentEnable("NslWifiChannel", LOG_ALL);

  // command
  CommandLine cmd;
  cmd.AddValue("duration", "Simulate duration", duration);
  cmd.AddValue("realtime", "Enable realtime mode", realtime);
  cmd.AddValue("jamming", "Enable jamming mode", jamming);
  cmd.Parse(argc, argv);

  // realtime
  if (realtime)
  {
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
  }

  // global
  IoTNet::world = CreateObject<IoTNet>();

  // server
  IoTNetServer server("server", "10.1.1.0", "255.255.255.0", Vector(50.0, 5.0, 0.0));

  // wifi network
  IoTNetWifi wifiPB("wifi-phong-bep", "10.1.3.0", "255.255.255.0", Vector(50.0, 40.0, 0.0));
  Ptr<IoTNetNode> s1 = wifiPB.Create("may-nuoc-nong", Vector(55.0, 25.0, 0.0));
  Ptr<IoTNetNode> s2 = wifiPB.Create("bep", Vector(40.0, 30.0, 0.0));
  Ptr<IoTNetNode> s3 = wifiPB.Create("may-giat", Vector(45.0, 55.0, 0.0));
  Ptr<IoTNetNode> s4 = wifiPB.Create("tu-lanh", Vector(55.0, 55.0, 0.0));
  Ptr<IoTNetNode> j = wifiPB.Create("jammer", Vector(50.0, 50.0, 0.0));

  IoTNetWifi wifiPK("wifi-phong-khach", "10.1.4.0", "255.255.255.0", Vector(15.0, 35.0, 0.0));
  wifiPK.Create("bong-den", Vector(20.0, 20.0, 0.0));
  wifiPK.Create("may-lanh", Vector(5.0, 25.0, 0.0));
  wifiPK.Create("may-hut-bui", Vector(5.0, 45.0, 0.0));
  wifiPK.Create("tivi", Vector(15.0, 50.0, 0.0));

  IoTNetWifi wifiPN("wifi-phong-ngu", "10.1.5.0", "255.255.255.0", Vector(85.0, 35.0, 0.0));
  wifiPN.Create("rem-cua", Vector(85.0, 25.0, 0.0));
  wifiPN.Create("den-ban", Vector(75.0, 45.0, 0.0));
  wifiPN.Create("dong-ho", Vector(90.0, 50.0, 0.0));

  // old integrate
  NodeContainer c, networkNodes;
  c.Add(s1->node);
  c.Add(s2->node);
  c.Add(j->node);
  networkNodes.Add(s1->node);
  networkNodes.Add(s2->node);

  // router
  IoTNetRouter router("router", "10.1.2.0", "255.255.255.0", Vector(50.0, 15.0, 0.0));
  server.Add(router.GetNode());

  NodeContainer apNodes;
  apNodes.Add(wifiPK.GetAp()->node);
  apNodes.Add(wifiPB.GetAp()->node);
  apNodes.Add(wifiPN.GetAp()->node);
  router.Add(apNodes);

  IoTNet::world->address = server.GetAddress();

  NetDeviceContainer devices, jammerNetdevice;
  devices.Add(s1->device);
  devices.Add(s2->device);
  jammerNetdevice.Add(j->device);

  // wireless utility
  WirelessModuleUtilityHelper utilityHelper;
  std::vector<std::string> AllInclusionList;
  std::vector<std::string> AllExclusionList;
  utilityHelper.SetInclusionList(AllInclusionList);
  utilityHelper.SetExclusionList(AllExclusionList);
  WirelessModuleUtilityContainer utilities = utilityHelper.InstallAll();

  Ptr<WirelessModuleUtility> utilitySend = utilities.Get(0);    // node 0
  Ptr<WirelessModuleUtility> utilityReceive = utilities.Get(1); // node 1

  // jammer
  JammerHelper jammerHelper;
  jammerHelper.SetJammerType("ns3::ReactiveJammer");
  jammerHelper.Set("ReactiveJammerRxTimeout", TimeValue(Seconds(2.0)));
  jammerHelper.Set("ReactiveJammerReactionStrategy",
                   UintegerValue(ReactiveJammer::FIXED_PROBABILITY));
  JammerContainer jammers = jammerHelper.Install(c.Get(nodeCount - 1));
  Ptr<Jammer> jammerPtr = jammers.Get(0);

  // jamming mitigation
  // JammingMitigationHelper mitigationHelper;
  // mitigationHelper.SetJammingMitigationType("ns3::MitigateByChannelHop");
  // mitigationHelper.Set("MitigateByChannelHopChannelHopDelay",
  //                      TimeValue(Seconds(0.0)));
  // mitigationHelper.Set("MitigateByChannelHopDetectionMethod",
  //                      UintegerValue(MitigateByChannelHop::PDR_ONLY));
  // JammingMitigationContainer mitigators = mitigationHelper.Install(networkNodes); // normal
  // Ptr<JammingMitigation> mitigationPtr = mitigators.Get(1);                       // node 1
  // Ptr<JammingMitigation> mitigationPtr2 = mitigators.Get(0);                      // node 0

  // Install
  IoTNet::world->Install();

  AnimationInterface anim("output/iotnet-anim.xml");
  anim.EnablePacketMetadata();
  if (!jamming)
  {
    anim.SetBackgroundImage("/home/haova/Downloads/iotnet-normal-bg.png", 0, 0, 0.05, 0.05, 1);
  }
  else
  {
    anim.SetBackgroundImage("/home/haova/Downloads/iotnet-malicious-bg.png", 0, 0, 0.05, 0.05, 1);
  }
  // IoTNet::world->UpdateAnimationInterface(anim);

  // schedule
  if (jamming)
  {
    Simulator::Schedule(Seconds(2), &ns3::Jammer::StartJammer, jammerPtr); // start jammer at 2s
  }

  // Simulator::Schedule(Seconds(0.1), NodePdr, utilitySend, duration);
  // Simulator::Schedule(Seconds(0.1), NodePdr, utilityReceive, duration);

  // Simulator::Schedule(Seconds(0.1), NodeRss, utilitySend, duration);
  // Simulator::Schedule(Seconds(0.1), NodeRss, utilityReceive, duration);

  // Simulator::Schedule(Seconds(startTime), &GenerateTraffic, source,
  //                     PpacketSize, networkNodes.Get(0), numPackets,
  //                     interPacketInterval);

  // simulation
  NS_LOG_UNCOND(">> Start simulation");
  Simulator::Stop(Seconds(duration));
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