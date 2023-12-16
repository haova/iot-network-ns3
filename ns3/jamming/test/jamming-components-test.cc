/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: He Wu <mdzz@u.washington.edu>
 */

// ns3
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/nstime.h"
#include "ns3/type-id.h"
#include "ns3/address.h"
// wifi
#include "ns3/nsl-wifi-helper.h"
#include "ns3/nqos-wifi-mac-helper.h"
// mobility
#include "ns3/mobility-helper.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/position-allocator.h"
// Internet
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
// energy
#include "ns3/basic-energy-source.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/wifi-radio-energy-model-helper.h"
// jamming
#include "ns3/jammer-helper.h"
#include "ns3/jamming-mitigation-helper.h"
#include "ns3/constant-jammer.h"
#include "ns3/random-jammer.h"
#include "ns3/eavesdropper-jammer.h"
#include "ns3/reactive-jammer.h"
#include "ns3/mitigate-by-channel-hop.h"
// other
#include <math.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("JammingComponentsTestSuite");

// -------------------------------------------------------------------------- //

/**
 * \brief Traffic generator.
 * \param socket Pointer to socket.
 * \param pktSize Packet size.
 * \param n Pointer to node.
 * \param pktCount Number of packets to generate.
 * \param pktInterval Packet sending interval.
 */
static void
GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, Ptr<Node> n,
                 uint32_t pktCount, Time pktInterval)
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic, socket, pktSize, n,
                           pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

/**
 * \brief Packet receiving sink.
 * \param socket Pointer to socket.
 */
void
ReceivePacket (Ptr<Socket> socket)
{
  Address addr;
  socket->GetSockName (addr);
  InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (addr);
  NS_LOG_UNCOND ("--\nReceived one packet! Socket: "<< iaddr.GetIpv4 () <<
                 " port: " << iaddr.GetPort () << "\n--");
}

// -------------------------------------------------------------------------- //

/**
 * Test case of installing different types of jammers onto a node. Also tests
 * hidden installation of WirelessModuleUtility object.
 */
class JammerTypeTest : public TestCase
{
public:
  JammerTypeTest ();
  virtual ~JammerTypeTest ();

private:
  void DoRun (void);

  /**
   * \brief Install and run different types of jammer.
   *
   * \param jammerType Type name of jammer class.
   * \returns False if no error occurs.
   */
  bool InstallJammer (std::string jammerType);

private:
  double m_simTimeS;
  double m_jammerStartTimeS;
};

JammerTypeTest::JammerTypeTest ()
  : TestCase ("Test of installing & running jammers.")
{
  m_simTimeS = 10.0;        // simulate for 10.0 seconds
  m_jammerStartTimeS = 5.0; // jammer starts at 5.0 seconds
}

JammerTypeTest::~JammerTypeTest ()
{
}

void
JammerTypeTest::DoRun (void)
{
  NS_TEST_ASSERT_MSG_EQ (InstallJammer ("ns3::ConstantJammer"), false,
                         "Failed to install ConstantJammer!");
  NS_TEST_ASSERT_MSG_EQ (InstallJammer ("ns3::RandomJammer"), false,
                         "Failed to install RandomJammer!");
  NS_TEST_ASSERT_MSG_EQ (InstallJammer ("ns3::EavesdropperJammer"), false,
                         "Failed to install EavesdropperJammer!");
  NS_TEST_ASSERT_MSG_EQ (InstallJammer ("ns3::ReactiveJammer"), false,
                         "Failed to install ReactiveJammer!");
}

bool
JammerTypeTest::InstallJammer (std::string jammerType)
{
  // create node
  Ptr<Node> node = CreateObject<Node> ();

  /*
   * Create and install wifi.
   */
  std::string phyMode ("DsssRate1Mbps");
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                      StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                      StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  NslWifiPhyHelper wifiPhy =  NslWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (NslWifiPhyHelper::DLT_IEEE802_11_RADIO);

  NslWifiChannelHelper wifiChannel ;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue(phyMode),
                                "ControlMode", StringValue(phyMode));
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, node);

  /*
   * Create and install mobility.
   */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator> ();
  posAlloc->Add (Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (posAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (node);

  /*
   * Create and install energy source & device energy model
   */
  BasicEnergySourceHelper basicSourceHelper;
  EnergySourceContainer energySources = basicSourceHelper.Install (node);
  WifiRadioEnergyModelHelper radioEnergyHelper;
  DeviceEnergyModelContainer deviceModels =
    radioEnergyHelper.Install (devices, energySources);

  /*
   * Create and install jammer.
   */
  JammerHelper jammerHelper;
  jammerHelper.SetJammerType (jammerType);
  JammerContainer jammers = jammerHelper.Install (node);
  //jammerHelper.EnableLogComponents ();

  // start jammers at 7.0 seconds
  JammerContainer::Iterator i;
  for (i = jammers.Begin (); i != jammers.End (); i++)
    {
      Simulator::Schedule (Seconds (m_jammerStartTimeS),
                           &ns3::Jammer::StartJammer,
                           *i);
    }

  /*
   * Run simulation.
   */
  Simulator::Stop (Seconds (m_simTimeS));
  Simulator::Run ();
  Simulator::Destroy ();

  return false; // all is good
}

// -------------------------------------------------------------------------- //

/**
 * Test case of installing different types of jamming mitigation classes onto a
 * node. Also tests hidden installation of WirelessModuleUtility object.
 */
class JammingMitigationTypeTest : public TestCase
{
public:
  JammingMitigationTypeTest ();
  virtual ~JammingMitigationTypeTest ();

private:
  void DoRun (void);

  /**
   * \brief Install and run different types of jamming mitigation class.
   *
   * \param mitigationType Type name of jamming mitigation class.
   * \returns False if no error occurs.
   */
  bool InstallMitigation (std::string mitigationType);

private:
  double m_simTimeS;
  double m_mitigationStartTimeS;
};

JammingMitigationTypeTest::JammingMitigationTypeTest ()
  : TestCase ("Test of installing & running of jamming mitigation.")
{
  m_simTimeS = 10.0;            // simulate for 10.0 seconds
  m_mitigationStartTimeS = 5.0; // jammer starts at 5.0 seconds
}

JammingMitigationTypeTest::~JammingMitigationTypeTest ()
{
}

void
JammingMitigationTypeTest::DoRun (void)
{
  NS_TEST_ASSERT_MSG_EQ (InstallMitigation ("ns3::MitigateByChannelHop"), false,
                         "Failed to install MitigateByChannelHop!");
}

bool
JammingMitigationTypeTest::InstallMitigation (std::string mitigationType)
{
  // create node
  Ptr<Node> node = CreateObject<Node> ();

  /*
   * Create and install wifi.
   */
  std::string phyMode ("DsssRate1Mbps");
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                      StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                      StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  NslWifiPhyHelper wifiPhy =  NslWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (NslWifiPhyHelper::DLT_IEEE802_11_RADIO);

  NslWifiChannelHelper wifiChannel ;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue(phyMode),
                                "ControlMode", StringValue(phyMode));
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, node);

  /*
   * Create and install mobility.
   */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator> ();
  posAlloc->Add (Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (posAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (node);

  /*
   * Create and install energy source & device energy model
   */
  BasicEnergySourceHelper basicSourceHelper;
  EnergySourceContainer energySources = basicSourceHelper.Install (node);
  WifiRadioEnergyModelHelper radioEnergyHelper;
  DeviceEnergyModelContainer deviceModels =
    radioEnergyHelper.Install (devices, energySources);

  /*
   * Create and install jamming mitigation.
   */
  JammingMitigationHelper mitigationHelper;
  mitigationHelper.SetJammingMitigationType (mitigationType);
  JammingMitigationContainer mitigators = mitigationHelper.Install (node);
  //mitigationHelper.EnableLogComponents ();

  // start jamming mitigation on all nodes
  JammingMitigationContainer::Iterator i;
  for (i = mitigators.Begin (); i != mitigators.End (); i++)
    {
      Simulator::Schedule (Seconds (m_mitigationStartTimeS),
                           &ns3::JammingMitigation::StartMitigation,
                           *i);
    }

  /*
   * Run simulation.
   */
  Simulator::Stop (Seconds (m_simTimeS));
  Simulator::Run ();
  Simulator::Destroy ();

  return false; // all is good
}

// -------------------------------------------------------------------------- //

/**
 * Test case of installing different types of WirelessModuleUtility class onto
 * nodes. Values recorded by WirelessModuleUtility are checked in this test:
 *  1.  RSS. Using Friis propagation loss mode.
 *  2.  Throughput (TX & RX)
 *  3.  Total bytes (TX & RX)
 *  4.  PDR
 *
 * Note that we never move the node out of range. Therefore the receiving node
 * should always be able to receive the packet.
 */
class WirelessModuleUtilityTest : public TestCase
{
public:
  WirelessModuleUtilityTest ();
  virtual ~WirelessModuleUtilityTest ();

  /**
   * \brief RSS trace function for receiver node.
   * \param oldValue Old RSS value.
   * \param rss New RSS value.
   */
  void NodeRss (double oldValue, double rss);

  void PacketRss (double oldValue, double newValue);

private:
  void DoRun (void);

  /**
   * \brief Run simulation of 2 nodes with specified distance.
   *
   * \param distance Distance between 2 nodes.
   * \returns False if no error occurs.
   */
  bool SimulateTwoNodes (double distance);

  /**
   * \brief Calculates RSS using Friis propagation loss model.
   *
   * \param distance Distance between sender and receiver.
   * \returns RSS, in watts. Both Tx & Rx gain are included in the result.
   *
   * Taken from FriisPropagationLossModel::DoCalcRxPower. Note that RSS is no
   * lower then the noise floor.
   */
  double CalculateRssFriis (double distance);

  /**
   * \brief Converts dBm to Watts.
   *
   * \param dBm dBm.
   * \return Watts.
   */
  double DbmToW (double dBm) const;

private:
  double m_simTimeS;
  double m_maxDistance;     // maximum distance
  double m_distanceStep;    // step size for distance increase
  /**
   * Cutoff distance, no packet can be received beyond or at such distance.
   */
  double m_cutoffDistance;
  uint64_t m_packetSize;    // size of each packet, in bytes
  uint64_t m_numPackets;    // number of packets to send
  double m_interPacketIntervalS; // interval between each packet send
  double m_tolerance;       // tolerance for double values

  double m_txPowerDbm;      // TX power, in dBm
  double m_txGainDbm;       // TX gain, in dBm
  double m_rxGainDbm;       // RX gain, in dBm
  /**
   * The utility class records total size of a packet, instead of only payload
   * sizes. When calculating throughput in this test case, we will have to add
   * the packet overhead manually.
   */
  double m_udpPacketOverhead;

  double m_lambda;      // wavelength for Friis propagation loss model
  double PI;            // PI
  double m_minDistance; // minimum distance for Friis propagation loss model
  double m_systemLoss;  // system loss for Friis propagation loss model

  /**
   * Noise floor for this test, in Watts.
   */
  double m_noiseFloor;
  double m_maxRssW;     // maximum RSS during simulation, in Watts=
  double m_avgPktRssW;  // average packet RSS
};

WirelessModuleUtilityTest::WirelessModuleUtilityTest ()
  : TestCase ("Test of installing & running of WirelessModuleUtility.")
{
  m_simTimeS = 20.0;      // simulate for 20.0 seconds
  m_maxDistance = 10.0;   // go up to 10 meters
  m_distanceStep = 1.0;   // go up 1 meter per step
  m_cutoffDistance = 5000.0;  //
  m_numPackets = 1000;
  m_packetSize = 200;
  m_interPacketIntervalS = 1.0;

  m_txPowerDbm = 16.0206;
  m_txGainDbm = 1.0;      // magic number
  m_rxGainDbm = 0.0;

  /*
   * epsilon = 2.22045e-16
   */
  m_tolerance = std::numeric_limits<double>::epsilon ();

  m_udpPacketOverhead = 64; // UDP header/trailer overhead

  m_lambda = 300000000.0 / 5.150e9;
  PI = 3.14159265358979323846;
  m_minDistance = 0.5;
  m_systemLoss = 1.0;

  /*
   * Noise floor = Boltzmann Constant * 290.0 * Bandwidth * Noise Figure
   * Where in this test case:
   *    Boltzmann Constant = 1.3803e-23
   *    Bandwidth of test setup = 22000000
   *    Noise Figure = 5.01187
   * value = 4.41361e-13
   */
  m_noiseFloor = 1.3803e-23 * 290.0 * 22000000 * 5.01187;

  m_maxRssW = 0;
  m_avgPktRssW = 0;
}

WirelessModuleUtilityTest::~WirelessModuleUtilityTest ()
{
}

void
WirelessModuleUtilityTest::DoRun (void)
{
  // normal test
  for (double distance = m_distanceStep; distance <= m_maxDistance;
       distance += m_distanceStep)
    {
      NS_TEST_ASSERT_MSG_EQ (SimulateTwoNodes (distance), false,
                             "Failed WirelessModuleUtility normal test");
    }

  // 0 PDR test
  NS_TEST_ASSERT_MSG_EQ (SimulateTwoNodes (m_cutoffDistance + 1.0), false,
                         "Failed WirelessModuleUtility 0 PDR test");
}

bool
WirelessModuleUtilityTest::SimulateTwoNodes (double distance)
{
  // reset RSS value
  m_maxRssW = 0;
  m_avgPktRssW = 0;

  // create 2 nodes
  NodeContainer c;
  c.Create (2);

  /*
   * Create and install wifi.
   */
  std::string phyMode ("DsssRate1Mbps");
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                      StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                      StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  NslWifiPhyHelper wifiPhy =  NslWifiPhyHelper::Default ();
  wifiPhy.Set ("NslRxGain", DoubleValue (m_rxGainDbm));
  wifiPhy.Set ("NslTxGain", DoubleValue (m_txGainDbm));
  wifiPhy.Set ("NslTxPowerLevels", UintegerValue (1));
  wifiPhy.Set ("NslTxPowerEnd", DoubleValue (m_txPowerDbm));
  wifiPhy.Set ("NslTxPowerStart", DoubleValue (m_txPowerDbm));
  wifiPhy.Set ("NslCcaMode1Threshold", DoubleValue (0.0));
  wifiPhy.SetPcapDataLinkType (NslWifiPhyHelper::DLT_IEEE802_11_RADIO);

  NslWifiChannelHelper wifiChannel ;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue(phyMode),
                                "ControlMode", StringValue(phyMode));
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

  /*
   * Create and install mobility.
   */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator> ();
  posAlloc->Add (Vector (0.0, 0.0, 0.0));
  posAlloc->Add (Vector (distance, 0.0, 0.0));  // second node is on x-axis
  mobility.SetPositionAllocator (posAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

  /*
   * Create and install energy source & device energy model
   */
  BasicEnergySourceHelper basicSourceHelper;
  EnergySourceContainer energySources = basicSourceHelper.Install (c);
  WifiRadioEnergyModelHelper radioEnergyHelper;
  DeviceEnergyModelContainer deviceModels =
    radioEnergyHelper.Install (devices, energySources);

  /*
   * Create and install utility.
   */
  WirelessModuleUtilityHelper utilityHelper;
  // set inclusion list to record only UDP packets
  std::vector<std::string> InclusionList;
  InclusionList.push_back ("ns3::UdpHeader");
  // set exclusion list to exclude OLSR packets
  std::vector<std::string> ExclusionList;
  ExclusionList.push_back ("ns3::olsr::PacketHeader");
  utilityHelper.SetInclusionList (InclusionList);
  utilityHelper.SetExclusionList (ExclusionList);
  // set update interval
  utilityHelper.Set ("RssUpdateInterval", TimeValue (MilliSeconds (100)));
  WirelessModuleUtilityContainer utilities = utilityHelper.InstallAll ();
  Ptr<WirelessModuleUtility> utilSend = utilities.Get (0);
  Ptr<WirelessModuleUtility> utilRecv = utilities.Get (1);
  if ((utilSend == NULL) || (utilRecv == NULL))
    {
      NS_LOG_UNCOND ("Failed to aggregate utility onto nodes!");
      return true;
    }
  // connect trace source
  Callback<void, double, double> rssTraceCallback;
  rssTraceCallback = MakeCallback (&WirelessModuleUtilityTest::NodeRss, this);
  utilRecv->TraceConnectWithoutContext ("Rss", rssTraceCallback);
  Callback<void, double, double> packetRssTraceCallback;
  packetRssTraceCallback = MakeCallback (&WirelessModuleUtilityTest::PacketRss, this);
  utilRecv->TraceConnectWithoutContext ("PacketRss", packetRssTraceCallback);

  /*
   * Create and install Internet stack (with OLSR enabled to generate some OLSR
   * traffic), UDP socket.
   */
  // enable OLSR
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;
  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);
  // install Internet stack
  InternetStackHelper internet;
  internet.SetRoutingHelper (list);
  internet.Install (c);
  // assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  // node 1, receiver
  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (1), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
  // node 0, sender
  Ptr<Socket> source = Socket::CreateSocket (c.Get (0), tid);
  InetSocketAddress remote = InetSocketAddress (Ipv4Address::GetBroadcast (), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);

  /*
   * Generate some traffic during simulation. Start traffic at the beginning of
   * simulation. We do not have to wait for OLSR to converge since there are 2
   * nodes only in this simulation. We needed only OLSR traffic to test ability
   * of filtering packets in WirelessModuleUtility.
   */
  Simulator::Schedule (Seconds (0.0), &GenerateTraffic, source, m_packetSize,
                       c.Get (0), m_numPackets, Seconds (m_interPacketIntervalS));

  /*
   * Run simulation.
   */
  Simulator::Stop (Seconds (m_simTimeS));
  Simulator::Run ();
  Simulator::Destroy ();

  /*
   * Simulation results.
   */
  uint64_t actualTxTotalBytes = utilSend->GetTotalBytesTx (); // sender
  uint64_t actualRxTotalBytes = utilRecv->GetTotalBytesRx (); // receiver
  double actualTxThroughput = utilSend->GetTxThroughput (); // sender
  double actualRxThroughput = utilRecv->GetRxThroughput (); // receiver
  double actualPdr = utilRecv->GetPdr (); // receiver

  NS_LOG_UNCOND ("\nDistance = " << distance);
  NS_LOG_UNCOND ("Actual TX total bytes = " << actualTxTotalBytes);
  NS_LOG_UNCOND ("Actual RX total bytes = " << actualRxTotalBytes);
  NS_LOG_UNCOND ("Actual TX throughput = " << actualTxThroughput);
  NS_LOG_UNCOND ("Actual RX throughput = " << actualRxThroughput);
  NS_LOG_UNCOND ("Actual PDR = " << actualPdr);
  NS_LOG_UNCOND ("Actual RSS = " << m_maxRssW);
  NS_LOG_UNCOND ("Actual Average Packet RSS = " << m_avgPktRssW << "\n");

  /*
   * Calculated results.
   */
  uint64_t totalNumPackets = (uint64_t) floor (m_simTimeS / m_interPacketIntervalS);
  totalNumPackets = (totalNumPackets > m_numPackets) ? m_numPackets : totalNumPackets;

  uint64_t actualPacketSize = m_packetSize + m_udpPacketOverhead;
  uint64_t calculatedTxTotalBytes = actualPacketSize * totalNumPackets;
  uint64_t calculatedRxTotalBytes = actualPacketSize * totalNumPackets;
  // when distance > cutoff distance, RX total bytes will drop to 0
  calculatedRxTotalBytes = (distance >= m_cutoffDistance) ? 0 : calculatedRxTotalBytes;
  double calculatedTxThroughput = calculatedTxTotalBytes * 8 / m_simTimeS;
  double calculatedRxThroughput = calculatedRxTotalBytes * 8 / m_simTimeS;
  // when distance > cutoff distance, PDR will drop to 0
  double calculatedPdr = (distance >= m_cutoffDistance) ? 0 : 1;
  double calculatedRss = CalculateRssFriis (distance);

  NS_LOG_UNCOND ("Calculated TX total bytes = " << calculatedTxTotalBytes);
  NS_LOG_UNCOND ("Calculated RX total bytes = " << calculatedRxTotalBytes);
  NS_LOG_UNCOND ("Calculated TX throughput = " << calculatedTxThroughput);
  NS_LOG_UNCOND ("Calculated RX throughput = " << calculatedRxThroughput);
  NS_LOG_UNCOND ("Calculated PDR = " << calculatedPdr);
  NS_LOG_UNCOND ("Calculated RSS = " << calculatedRss << "\n");

  NS_LOG_UNCOND ("RSS difference = " << calculatedRss - m_maxRssW);
  NS_LOG_UNCOND ("Packet RSS difference = " << calculatedRss - m_avgPktRssW);

  /*
   * Check results.
   */
  NS_TEST_ASSERT_MSG_EQ_RETURNS_BOOL (calculatedTxTotalBytes, actualTxTotalBytes,
                                      "Incorrect Tx Total Bytes!");
  NS_TEST_ASSERT_MSG_EQ_RETURNS_BOOL (calculatedRxTotalBytes, actualRxTotalBytes,
                                      "Incorrect Rx Total Bytes!");
  NS_TEST_ASSERT_MSG_EQ_RETURNS_BOOL (calculatedTxThroughput, actualTxThroughput,
                                      "Incorrect Tx Throughput!");
  NS_TEST_ASSERT_MSG_EQ_RETURNS_BOOL (calculatedRxThroughput, actualRxThroughput,
                                      "Incorrect Rx Throughput!");
  NS_TEST_ASSERT_MSG_EQ_TOL_RETURNS_BOOL (calculatedPdr, actualPdr, m_tolerance,
                                          "Incorrect PDR!");
  NS_TEST_ASSERT_MSG_EQ_TOL_RETURNS_BOOL (calculatedRss, m_maxRssW, m_tolerance,
                                          "Incorrect RSS!");
  NS_TEST_ASSERT_MSG_EQ_TOL_RETURNS_BOOL (calculatedRss, m_avgPktRssW, m_tolerance,
                                          "Incorrect Packet RSS!");

  return false; // all is good
}

double
WirelessModuleUtilityTest::CalculateRssFriis (double distance)
{
  double txPowerDbm = m_txGainDbm + m_txPowerDbm;
  if (distance <= m_minDistance)
    {
      return m_noiseFloor + DbmToW (txPowerDbm + m_rxGainDbm);
    }
  double numerator = m_lambda * m_lambda;
  double denominator = 16 * PI * PI * distance * distance * m_systemLoss;
  double pr = 10 * log10 (numerator / denominator);
  double rssDbm = txPowerDbm + pr + m_rxGainDbm;
  return m_noiseFloor + DbmToW (rssDbm);
}

double
WirelessModuleUtilityTest::DbmToW (double dBm) const
{
  double mW = pow (10.0, dBm / 10.0);
  return mW / 1000.0;
}

void
WirelessModuleUtilityTest::NodeRss (double oldValue, double rss)
{
  if (rss > m_maxRssW)
    {
      m_maxRssW = rss;
    }
}

void
WirelessModuleUtilityTest::PacketRss (double oldValue, double newValue)
{
  if (newValue > m_avgPktRssW)
    {
      m_avgPktRssW = newValue;
    }
}

// -------------------------------------------------------------------------- //

/**
 * Test suite for Jammer, JammingMitigation & WirelessModuleUtility components.
 */
class JammingComponentTestSuite : public TestSuite
{
public:
  JammingComponentTestSuite ();
};

JammingComponentTestSuite::JammingComponentTestSuite ()
  : TestSuite ("jamming-components-test", UNIT)
{
  AddTestCase (new JammerTypeTest);
  AddTestCase (new JammingMitigationTypeTest);
  AddTestCase (new WirelessModuleUtilityTest);
}

// create an instance of the test suite
JammingComponentTestSuite g_jammingComponentsTestSuite;

} // namespace ns3
