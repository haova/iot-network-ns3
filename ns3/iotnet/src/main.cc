#include "ns3/core-module.h"
#include "ns3/jamming-module.h"
#include "ns3/mobility-module.h"
#include "ns3/energy-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"

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

int main(int argc, char *argv[])
{
  NS_LOG_UNCOND("Setting something up");

  // settings
  std::string phyMode("DsssRate1Mbps");
  int nodeCount = 3;           // 2 normal [0, 1], 1 jammer [2]
  double distanceToRx = 10.0;  // meters
  double TimeSimulation = 5;   // seconds
  double startTime = 0.0;      // seconds
  double interval = 0.1;       // seconds
  uint32_t PpacketSize = 1000; // bytes
  uint32_t numPackets = 1;     // number of packets to send

  Time interPacketInterval = Seconds(interval);

  // logger
  // LogComponentEnable("MacLow", LOG_ALL);
  // LogComponentEnable("NslWifiChannel", LOG_ALL);

  // command
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // node define:
  NodeContainer c;
  c.Create(nodeCount);

  NodeContainer networkNodes;
  networkNodes.Add(c.Get(0));
  networkNodes.Add(c.Get(1));

  // wifi
  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

  // wifi phy
  NslWifiPhyHelper wifiPhy = NslWifiPhyHelper::Default();

  // wifi channel
  NslWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel(wifiChannel.Create());

  // mac layer
  WifiMacHelper wifiMac = WifiMacHelper();
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode", StringValue(phyMode));

  // adhoc network
  wifiMac.SetType("ns3::AdhocWifiMac");

  // phy + mac + normal
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, networkNodes);

  // phy + mac + jammer
  NetDeviceContainer jammerNetdevice = wifi.Install(wifiPhy, wifiMac, c.Get(nodeCount - 1));

  // mobility
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));                         // node 0
  positionAlloc->Add(Vector(distanceToRx, 0.1 * distanceToRx, 0.0)); // node 1
  positionAlloc->Add(Vector(2 * distanceToRx, 0.0, 0.0));            // node 2

  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(c);

  // energy
  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(25.0));
  EnergySourceContainer energySources = basicSourceHelper.Install(c);
  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));

  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install(devices, energySources);
  DeviceEnergyModelContainer jammerDeviceModels = radioEnergyHelper.Install(jammerNetdevice.Get(0), energySources.Get(nodeCount - 1));

  // wireless utility
  WirelessModuleUtilityHelper utilityHelper;
  std::vector<std::string> AllInclusionList;
  std::vector<std::string> AllExclusionList;
  AllInclusionList.push_back("ns3::UdpHeader");          // record only UdpHeader
  AllExclusionList.push_back("ns3::olsr::PacketHeader"); // ignore all olsr headers/trailers
  utilityHelper.SetInclusionList(AllInclusionList);
  utilityHelper.SetExclusionList(AllExclusionList);
  WirelessModuleUtilityContainer utilities = utilityHelper.InstallAll();

  Ptr<WirelessModuleUtility> utilitySend = utilities.Get(0); // node 0
  Ptr<WirelessModuleUtility> utilityPtr = utilities.Get(1);  // node 1

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

  // internet
  InternetStackHelper internet;
  internet.Install(networkNodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ipv4Interface = ipv4.Assign(devices);

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket(networkNodes.Get(1), tid); // node 1 - receiver
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
  recvSink->Bind(local);
  recvSink->SetRecvCallback(MakeCallback(&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket(networkNodes.Get(0), tid); // node 0 - sender
  InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetBroadcast(), 80);
  source->SetAllowBroadcast(true);
  source->Connect(remote);

  // schedule
  Simulator::Schedule(Seconds(startTime), &GenerateTraffic, source,
                      PpacketSize, networkNodes.Get(0), numPackets,
                      interPacketInterval);

  // simulation
  NS_LOG_UNCOND(">> Start simulation");
  Simulator::Stop(Seconds(TimeSimulation));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_UNCOND("<< Stop simulation");

  // information
  uint32_t actualPdr = utilityPtr->GetTotalPkts();      // node 1 - receiver
  uint32_t actualSend = utilitySend->GetTotalBytesTx(); // node 0 - sender

  NS_LOG_UNCOND("Actual Bytes Send = " << actualSend);
  NS_LOG_UNCOND("Actual PDR = " << actualPdr);

  return 0;
}