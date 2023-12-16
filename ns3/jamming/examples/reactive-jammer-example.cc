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
 * Author: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

/*
 * This example demonstrates how EnergyModel, Jammer, JammingMitigation,
 * WirelessModuleUtility and NslWifiPhy can be used together to simulate a
 * jammed wifi ad-hoc network.
 *
 * Network topology: 4 honest nodes + 1 jammer. One honest node will broadcast
 * UDP packets too all the other nodes. The jammer will try to jam node 3.
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/helper-module.h"
#include "ns3/mobility-module.h"
#include "ns3/contrib-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/energy-module.h"
#include "ns3/jamming-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("ReactiveJammerExample");

using namespace ns3;

/**
 * \brief Packet receiving sink.
 *
 * \param socket Pointer to socket.
 */
void
ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while (packet = socket->RecvFrom (from))
    {
      if (packet->GetSize () > 0)
        {
          InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (from);
          NS_LOG_UNCOND ("--\nReceived one packet! Socket: "<< iaddr.GetIpv4 ()
                         << " port: " << iaddr.GetPort () << " at time = " <<
                         Simulator::Now ().GetSeconds () << "\n--");
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
 * \brief Trace function for remaining energy at node.
 *
 * \param oldValue Old remaining energy value.
 * \param remainingEnergy New remaining energy value.
 */
void
RemainingEnergy (double oldValue, double remainingEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () <<
                 "s Current remaining energy = " << remainingEnergy << "J");
}

/**
 * \brief Trace function for total energy consumption at node.
 *
 * \param oldValue Old total energy consumption value.
 * \param totalEnergy New total energy consumption value.
 */
void
TotalEnergy (double oldValue, double totalEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () <<
                 "s Total energy consumed by radio = " << totalEnergy << "J");
}

/**
 * \brief Trace function for node RSS.
 *
 * \param oldValue Old RSS value.
 * \param rss New RSS value.
 */
void
NodeRss (double oldValue, double rss)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "s Node RSS = " << rss <<
                 "W");
}

/**
 * \brief Trace function for node PDR.
 *
 * \param oldValue Old PDR value.
 * \param pdr New PDR value.
 */
void
NodePdr (double oldValue, double pdr)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "s Node PDR = " << pdr);
}

/**
 * \brief Trace function for node RX throughput.
 *
 * \param oldValue Old RX throughput value.
 * \param rxThroughput New RX throughput value.
 */
void
NodeThroughputRx (double oldValue, double rxThroughput)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "s Node RX throughput = "
      << rxThroughput);
}

int
main (int argc, char *argv[])
{
  /*
  LogComponentEnable ("NslWifiPhy", LOG_LEVEL_DEBUG);
  LogComponentEnable ("EnergySource", LOG_LEVEL_DEBUG);
  LogComponentEnable ("BasicEnergySource", LOG_LEVEL_DEBUG);
  LogComponentEnable ("DeviceEnergyModel", LOG_LEVEL_DEBUG);
  LogComponentEnable ("WifiRadioEnergyModel", LOG_LEVEL_DEBUG);
  LogComponentEnable ("WirelessModuleUtility", LOG_LEVEL_DEBUG);
  LogComponentEnable ("JammerHelper", LOG_LEVEL_DEBUG);
  LogComponentEnable ("Jammer", LOG_LEVEL_DEBUG);
  LogComponentEnable ("ReactiveJammer", LOG_LEVEL_DEBUG);
  LogComponentEnable ("JammingMitigationHelper", LOG_LEVEL_DEBUG);
  LogComponentEnable ("JammingMitigation", LOG_LEVEL_DEBUG);
  LogComponentEnable ("MitigateByChannelHop", LOG_LEVEL_DEBUG);
   */

  std::string phyMode ("DsssRate1Mbps");
  double Prss = -80;            // dBm
  uint32_t PpacketSize = 200;   // bytes
  bool verbose = false;

  // simulation parameters
  uint32_t numPackets = 10000;  // number of packets to send
  double interval = 1;          // seconds
  double startTime = 0.0;       // seconds
  double distanceToRx = 10.0;   // meters
  /*
   * This is a magic number used to set the transmit power, based on other
   * configuration.
   */
  double offset = 81;

  CommandLine cmd;
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("Prss", "Intended primary RSS (dBm)", Prss);
  cmd.AddValue ("PpacketSize", "size of application packet sent", PpacketSize);
  cmd.AddValue ("numPackets", "Total number of packets to send", numPackets);
  cmd.AddValue ("startTime", "Simulation start time", startTime);
  cmd.AddValue ("distanceToRx", "X-Axis distance between nodes", distanceToRx);
  cmd.AddValue ("verbose", "Turn on all device log components", verbose);
  cmd.Parse (argc, argv);

  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                      StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                      StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  NodeContainer c;
  c.Create (5);     // create 4 nodes + 1 jammer
  NodeContainer networkNodes;
  networkNodes.Add (c.Get (0));
  networkNodes.Add (c.Get (1));
  networkNodes.Add (c.Get (2));
  networkNodes.Add (c.Get (3));

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  /** Wifi PHY **/
  /***************************************************************************/
  NslWifiPhyHelper wifiPhy = NslWifiPhyHelper::Default ();
  wifiPhy.Set ("NslRxGain", DoubleValue (-10));
  wifiPhy.Set ("NslTxGain", DoubleValue (offset + Prss));
  wifiPhy.Set ("NslCcaMode1Threshold", DoubleValue (0.0));
  /***************************************************************************/

  /** wifi channel **/
  NslWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  // create wifi channel
  Ptr<NslWifiChannel> wifiChannelPtr = wifiChannel.Create ();
  wifiPhy.SetChannel (wifiChannelPtr);

  /** MAC layer **/
  // Add a non-QoS upper MAC, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
      StringValue (phyMode), "ControlMode", StringValue (phyMode));
  // Set it to ad-hoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");

  /** install PHY + MAC **/
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, networkNodes);
  // install MAC & PHY onto jammer
  NetDeviceContainer jammerNetdevice = wifi.Install (wifiPhy, wifiMac, c.Get (4));

  /** mobility **/
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (distanceToRx, 0.1 * distanceToRx, 0.0));
  positionAlloc->Add (Vector (2 * distanceToRx, 0.0, 0.0));
  positionAlloc->Add (Vector (3 * distanceToRx, 0.1 * distanceToRx, 0.0));
  positionAlloc->Add (Vector (2 * distanceToRx, -0.5 * distanceToRx, 0.0)); // jammer location
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

  /** Energy Model **/
  /***************************************************************************/
  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  // configure energy source
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (0.1));
  // install on node
  EnergySourceContainer energySources = basicSourceHelper.Install (c);
  /* device energy model */
  WifiRadioEnergyModelHelper radioEnergyHelper;
  // configure radio energy model
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0174));
  // install on devices
  DeviceEnergyModelContainer deviceModels =
      radioEnergyHelper.Install (devices, energySources);
  DeviceEnergyModelContainer jammerDeviceModels =
      radioEnergyHelper.Install (jammerNetdevice.Get (0), energySources.Get (4));
  /***************************************************************************/

  /** WirelessModuleUtility **/
  /***************************************************************************/
  WirelessModuleUtilityHelper utilityHelper;
  // set inclusion/exclusion list for all nodes
  std::vector<std::string> AllInclusionList;
  AllInclusionList.push_back ("ns3::UdpHeader");          // record only UdpHeader
  std::vector<std::string> AllExclusionList;
  AllExclusionList.push_back ("ns3::olsr::PacketHeader"); // ignore all olsr headers/trailers
  // assign lists to helper
  utilityHelper.SetInclusionList (AllInclusionList);
  utilityHelper.SetExclusionList (AllExclusionList);
  // install on all nodes
  WirelessModuleUtilityContainer utilities = utilityHelper.InstallAll ();
  /***************************************************************************/

  /** Jammer **/
  /***************************************************************************/
  JammerHelper jammerHelper;
  // configure jammer type
  jammerHelper.SetJammerType ("ns3::ReactiveJammer");
  // set jammer parameters
  jammerHelper.Set ("ReactiveJammerRxTimeout", TimeValue (Seconds (2.0)));
  jammerHelper.Set ("ReactiveJammerReactionStrategy",
                    UintegerValue(ReactiveJammer::FIXED_PROBABILITY));
  // enable jammer reaction to jamming mitigation
  jammerHelper.Set ("ReactiveJammerReactToMitigation", UintegerValue(true));
  // install jammer
  JammerContainer jammers = jammerHelper.Install (c.Get (4));
  // Get pointer to Jammer
  Ptr<Jammer> jammerPtr = jammers.Get (0);
  // enable all jammer debug statements
  if (verbose)
    {
      jammerHelper.EnableLogComponents ();
    }
  /***************************************************************************/

  /** Internet stack **/
  InternetStackHelper internet;
  internet.Install (networkNodes);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (networkNodes.Get (3), tid);  // node 3, receiver
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (networkNodes.Get (0), tid);    // node 0, sender
  InetSocketAddress remote = InetSocketAddress (Ipv4Address::GetBroadcast (), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);

  /** connect trace sources **/
  /***************************************************************************/
  // all sources are connected to node 2
  // energy source
  Ptr<EnergySource> basicSourcePtr = energySources.Get (2);
  basicSourcePtr->TraceConnectWithoutContext ("RemainingEnergy",
                                              MakeCallback (&RemainingEnergy));
  // using honest node device energy model list
  Ptr<DeviceEnergyModel> basicRadioModelPtr = deviceModels.Get (2);
  basicRadioModelPtr->TraceConnectWithoutContext ("TotalEnergyConsumption",
                                                  MakeCallback (&TotalEnergy));
  // wireless module utility
  Ptr<WirelessModuleUtility> utilityPtr = utilities.Get (2);
  utilityPtr->TraceConnectWithoutContext ("Rss", MakeCallback (&NodeRss));
  utilityPtr->TraceConnectWithoutContext ("Pdr", MakeCallback (&NodePdr));
  /***************************************************************************/


  /** simulation setup **/
  // start traffic
  Simulator::Schedule (Seconds (startTime), &GenerateTraffic, source,
                       PpacketSize, networkNodes.Get (0), numPackets,
                       interPacketInterval);

  // start jammer at 7.0 seconds
  Simulator::Schedule (Seconds (startTime + 7.0), &ns3::Jammer::StartJammer,
                       jammerPtr);

  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
