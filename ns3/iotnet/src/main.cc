/** Implementation of jnetwork with jammer ***/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/energy-module.h"
#include "ns3/jamming-module.h"
#include "ns3/netanim-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE("ReactiveJammerExample");

using namespace ns3;

/**
 * \brief Packet receiving sink.
 *
 * \param socket Pointer to socket.
 */
void ReceivePacket(Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while (packet = socket->RecvFrom(from))
  {
    if (packet->GetSize() > 0)
    {
      InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(from);
      NS_LOG_UNCOND("--\nReceived one packet! Socket: " << iaddr.GetIpv4()
                                                        << " port: " << iaddr.GetPort() << " at time = " << Simulator::Now().GetSeconds() << "\n--");
    }
  }
}

/**
 * \brief Traffic generator.
 *
 * \param socket Pointer to socket.
 * \param pktSize Packet size.
 * \param n Pointer to node.
 * \param pktCount Number of packets to generate.
 * \param pktInterval Packet sending interval.
 */
static void
GenerateTraffic(Ptr<Socket> socket, uint32_t pktSize, Ptr<Node> n,
                uint32_t pktCount, Time pktInterval)
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

/**
 * \brief Trace function for remaining energy at node.
 *
 * \param oldValue Old remaining energy value.
 * \param remainingEnergy New remaining energy value.
 */
void RemainingEnergy(double oldValue, double remainingEnergy)
{
  NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s Current remaining energy = " << remainingEnergy << "J");
}

/**
 * \brief Trace function for total energy consumption at node.
 *
 * \param oldValue Old total energy consumption value.
 * \param totalEnergy New total energy consumption value.
 */
void TotalEnergy(double oldValue, double totalEnergy)
{
  NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s Total energy consumed by radio = " << totalEnergy << "J");
}

/**
 * \brief Trace function for node RSS.
 *
 * \param oldValue Old RSS value.
 * \param rss New RSS value.
 */
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

void GetChannel(Ptr<WirelessModuleUtility> utilitySend, double time)
{
  Time t = Simulator::Now();
  double channel = utilitySend->GetPhyLayerInfo().currentChannel;
  NS_LOG_UNCOND(Simulator::Now().GetSeconds() << " channel = " << channel);

  if (t.GetSeconds() < time)
  {
    Simulator::Schedule(Seconds(0.2), &GetChannel, utilitySend, time);
  }
}

/**
 * \brief Trace function for node RX throughput.
 *
 * \param oldValue Old RX throughput value.
 * \param rxThroughput New RX throughput value.
 */
void NodeThroughputRx(double oldValue, double rxThroughput)
{
  NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s Node RX throughput = "
                                              << rxThroughput);
}

int main(int argc, char *argv[])
{
  /** Simulation Parameters **/
  /***************************************************************************/

  std::string phyMode("DsssRate1Mbps");
  double Prss = -80;           // dBm
  uint32_t PpacketSize = 1000; // bytes
  bool verbose = false;

  uint32_t numPackets = 1000; // number of packets to send
  double interval = 0.1;      // seconds
  double startTime = 0.0;     // seconds
  double distanceToRx = 10.0; // meters
  // uint32_t numberNode = 5;
  double TimeSimulation = 3.2;

  CommandLine cmd;
  cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue("Prss", "Intended primary RSS (dBm)", Prss);
  cmd.AddValue("PpacketSize", "size of application packet sent", PpacketSize);
  cmd.AddValue("numPackets", "Total number of packets to send", numPackets);
  cmd.AddValue("startTime", "Simulation start time", startTime);
  cmd.AddValue("distanceToRx", "X-Axis distance between nodes", distanceToRx);
  cmd.AddValue("verbose", "Turn on all device log components", verbose);
  cmd.Parse(argc, argv);

  // LogComponentEnable ("NslWifiPhy", LOG_ALL);
  LogComponentEnable("NslWifiChannel", LOG_ALL);

  // LogComponentEnable ("MitigateByChannelHop", LOG_ALL);
  // LogComponentEnable ("ReactiveJammer", LOG_ALL);
  // LogComponentEnable ("WifiPhyStateHelper", LOG_ALL);
  // LogComponentEnable ("JammerHelper", LOG_ALL);
  // LogComponentEnable ("Jammer", LOG_ALL);
  // LogComponentEnable ("DetectionHelper", LOG_ALL);
  // LogComponentEnableAll(LOG_ALL);

  /***************************************************************************/

  /** Creation and installation of nodes **/
  /***************************************************************************/
  // Creation of nodes
  Time interPacketInterval = Seconds(interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                     StringValue("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     StringValue("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue(phyMode));
  /***************************************************************************/

  /** Creation and installation of nodes **/
  /***************************************************************************/
  // Creation of nodes
  NodeContainer c;
  c.Create(3); // create  1 jammer + x nodes
  NodeContainer networkNodes;
  networkNodes.Add(c.Get(0));
  networkNodes.Add(c.Get(1));
  // networkNodes.Add (c.Get (2));
  // networkNodes.Add (c.Get (3));

  // Wifi PHY layer
  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
  NslWifiPhyHelper wifiPhy = NslWifiPhyHelper::Default();

  // Wifi channel
  NslWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
  Ptr<NslWifiChannel> wifiChannelPtr = wifiChannel.Create();
  wifiPhy.SetChannel(wifiChannelPtr);

  // MAC layer
  WifiMacHelper wifiMac = WifiMacHelper();
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode", StringValue(phyMode));
  // Set it to ad-hoc mode
  wifiMac.SetType("ns3::AdhocWifiMac");

  // I PHY + MAC on legitimate node
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, networkNodes);
  // Install MAC & PHY onto jammer
  NetDeviceContainer jammerNetdevice = wifi.Install(wifiPhy, wifiMac, c.Get(2));
  /***************************************************************************/

  /** MobilityHelper **/
  /***************************************************************************/
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  // assign position to node
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(distanceToRx, 0.1 * distanceToRx, 0.0));
  positionAlloc->Add(Vector(2 * distanceToRx, 0.0, 0.0));
  // positionAlloc->Add (Vector (3 * distanceToRx, 0.1 * distanceToRx, 0.0));
  // positionAlloc->Add (Vector (2 * distanceToRx, -0.5 * distanceToRx, 0.0)); // jammer location
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(c);
  /***************************************************************************/

  /** Energy Model **/
  /***************************************************************************/
  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  // configure energy source
  basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(25.0));
  // install on node
  EnergySourceContainer energySources = basicSourceHelper.Install(c);
  /* device energy model */
  WifiRadioEnergyModelHelper radioEnergyHelper;
  // configure radio energy model
  radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));
  // install on devices
  DeviceEnergyModelContainer deviceModels =
      radioEnergyHelper.Install(devices, energySources);
  DeviceEnergyModelContainer jammerDeviceModels =
      radioEnergyHelper.Install(jammerNetdevice.Get(0), energySources.Get(2));
  /***************************************************************************/

  /** WirelessModuleUtility **/
  /***************************************************************************/
  WirelessModuleUtilityHelper utilityHelper;
  // set inclusion/exclusion list for all nodes
  std::vector<std::string> AllInclusionList;
  AllInclusionList.push_back("ns3::UdpHeader"); // record only UdpHeader
  std::vector<std::string> AllExclusionList;
  AllExclusionList.push_back("ns3::olsr::PacketHeader"); // ignore all olsr headers/trailers
  // assign lists to helper
  utilityHelper.SetInclusionList(AllInclusionList);
  utilityHelper.SetExclusionList(AllExclusionList);
  // install on all nodes
  WirelessModuleUtilityContainer utilities = utilityHelper.InstallAll();

  // link node to utilities
  Ptr<WirelessModuleUtility> utilitySend = utilities.Get(0);
  Ptr<WirelessModuleUtility> utilityPtr = utilities.Get(1);
  // Ptr<WirelessModuleUtility> utilityPtr2 = utilities.Get (2);
  // Ptr<WirelessModuleUtility> utilityPtr3 = utilities.Get (3);
  /***************************************************************************/

  /** Jammer **/
  /***************************************************************************/
  JammerHelper jammerHelper;
  // configure jammer type
  jammerHelper.SetJammerType("ns3::ReactiveJammer");
  // set jammer parameters
  // jammerHelper.Set("ConstantJammerJammingDuration",TimeValue (Seconds (0.05)));
  // install jammer
  JammerContainer jammers = jammerHelper.Install(c.Get(2));
  // Get pointer to Jammer
  Ptr<Jammer> jammerPtr = jammers.Get(0);
  /***************************************************************************/

  //**JammingMitigation**///
  /***************************************************************************/
  JammingMitigationHelper mitigationHelper;
  // configure mitigation type
  mitigationHelper.SetJammingMitigationType("ns3::MitigateByChannelHop");
  // configure mitigation parameters
  mitigationHelper.Set("MitigateByChannelHopChannelHopDelay",
                       TimeValue(Seconds(0.0)));
  mitigationHelper.Set("MitigateByChannelHopDetectionMethod",
                       UintegerValue(MitigateByChannelHop::PDR_ONLY));
  // install mitigation on honest nodes
  JammingMitigationContainer mitigators = mitigationHelper.Install(networkNodes);
  // get pointer to mitigation object from jamming mitigation container
  Ptr<JammingMitigation> mitigationPtr = mitigators.Get(1);
  Ptr<JammingMitigation> mitigationPtr2 = mitigators.Get(0);

  /***************************************************************************/

  /** Internet stack **/
  /***************************************************************************/
  InternetStackHelper internet;
  internet.Install(networkNodes);
  // Assign Ip Address
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ipv4Interface = ipv4.Assign(devices);
  // CreateSocket Receiver
  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket(networkNodes.Get(1), tid); // node 3, receiver
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
  recvSink->Bind(local);
  recvSink->SetRecvCallback(MakeCallback(&ReceivePacket));
  // CreateSocket Transmitter
  Ptr<Socket> source = Socket::CreateSocket(networkNodes.Get(0), tid); // node 0, sender
  InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetBroadcast(), 80);
  source->SetAllowBroadcast(true);
  source->Connect(remote);
  /***************************************************************************/

  /** Connect trace sources **/
  /***************************************************************************/
  // energy source
  Ptr<EnergySource> basicSourcePtr = energySources.Get(1);
  basicSourcePtr->TraceConnectWithoutContext("RemainingEnergy",
                                             MakeCallback(&RemainingEnergy));
  // using honest node device energy model list
  Ptr<DeviceEnergyModel> basicRadioModelPtr = deviceModels.Get(1);
  basicRadioModelPtr->TraceConnectWithoutContext("TotalEnergyConsumption",
                                                 MakeCallback(&TotalEnergy));
  /***************************************************************************/

  /** AnimationsInterface **/
  /***************************************************************************/
  AnimationInterface anim("ConstantRate2.xml");
  anim.EnablePacketMetadata(true);
  anim.EnableIpv4RouteTracking("tcp-Wifi-route.xml", Seconds(0), Seconds(10), Seconds(0.25));
  /***************************************************************************/

  /** Simulation Setup **/
  /***************************************************************************/
  Time t = Simulator::Now();
  std::cout << "Time hello: " << t.GetSeconds() << std::endl;
  Simulator::Schedule(Seconds(startTime), &GenerateTraffic, source,
                      PpacketSize, networkNodes.Get(0), numPackets,
                      interPacketInterval);

  // start jammer at 7.0 seconds

  Simulator::Schedule(Seconds(1), &ns3::Jammer::StartJammer,
                      jammerPtr);

  // Start Measure
  Simulator::Schedule(Seconds(0.1), NodePdr, utilityPtr, TimeSimulation);
  Simulator::Schedule(Seconds(0.1), NodePdr, utilitySend, TimeSimulation);

  Simulator::Schedule(Seconds(0.1), GetChannel, utilityPtr, TimeSimulation);
  Simulator::Schedule(Seconds(0.1), GetChannel, utilitySend, TimeSimulation);

  Simulator::Schedule(Seconds(0.1), NodeRss, utilityPtr, TimeSimulation);

  Simulator::Schedule(Seconds(1.2),
                      &ns3::JammingMitigation::StartMitigation,
                      mitigationPtr);

  Simulator::Schedule(Seconds(1.2),
                      &ns3::JammingMitigation::StartMitigation,
                      mitigationPtr2);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Stop(Seconds(TimeSimulation));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  uint32_t actualPdr = utilityPtr->GetTotalPkts(); // receiver
                                                   // uint32_t actualPdr2 = utilityPtr2->GetTotalPkts ();
  // double actualPdr3 = utilityPtr3->GetRss ();
  uint32_t actualSend = utilitySend->GetTotalPkts();
  // u_int64_t actualReceive = utilityPtr2->GetTotalBytesTx();
  // u_int64_t actualReceive2 = utilityPtr2->GetTotalBytesRx();

  NS_LOG_UNCOND("Actual Bytes Send = " << actualSend);
  // NS_LOG_UNCOND ("Actual Bytes Transmitter = " << actualReceive);
  // NS_LOG_UNCOND ("Actual Bytes Receive = " << actualReceive2);

  NS_LOG_UNCOND("Actual PDR = " << actualPdr);
  // NS_LOG_UNCOND ("Actual PDR = " << actualPdr2);
  // NS_LOG_UNCOND ("Actual Bytes Receive = " << actualPdr3);

  return 0;

  /***************************************************************************/
}