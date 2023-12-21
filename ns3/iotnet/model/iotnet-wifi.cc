/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "iotnet-wifi.h"

namespace ns3
{
  IoTNetWifi::IoTNetWifi(const std::string id, InternetStackHelper internet, const Ipv4Address network, const Ipv4Mask mask)
  {
    // config
    std::string phyMode("DsssRate1Mbps");
    m_id = id;

    // wifi
    m_wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

    // wifi phy
    m_wifiPhy = NslWifiPhyHelper::Default();

    // wifi channel
    m_wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    m_wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    m_wifiPhy.SetChannel(m_wifiChannel.Create());

    // mac layer
    m_wifiMac = WifiMacHelper();
    m_wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode", StringValue(phyMode));

    // adhoc network
    m_wifiMac.SetType("ns3::AdhocWifiMac");

    // mobility
    m_positionAlloc = CreateObject<ListPositionAllocator>();
    m_mobility.SetPositionAllocator(m_positionAlloc);
    m_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // energy
    m_basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(25.0));
    m_radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));

    // ip
    m_internet = internet;
    m_ipv4.SetBase(network, mask);
  }

  IoTNetNodePack IoTNetWifi::Create(std::string id, Vector position)
  {
    IoTNetNodePack pack;
    pack.id = id;

    // create node
    pack.node.Create(1);
    m_allNodes.Add(pack.node);

    // mobility
    m_positionAlloc->Add(position);
    m_mobility.Install(pack.node);

    // device = phy + mac + node
    pack.device = m_wifi.Install(m_wifiPhy, m_wifiMac, pack.node);

    // energy
    EnergySourceContainer energySource = m_basicSourceHelper.Install(pack.node);
    DeviceEnergyModelContainer deviceEnergyModel = m_radioEnergyHelper.Install(pack.device, energySource);

    // ip
    m_internet.Install(pack.node);
    Ipv4InterfaceContainer ipv4Interface = m_ipv4.Assign(pack.device);

    return pack;
  }
}