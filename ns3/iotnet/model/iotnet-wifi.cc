/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "iotnet-wifi.h"

namespace ns3
{
  IoTNetWifi::IoTNetWifi(const std::string id, const Ipv4Address network, const Ipv4Mask mask)
  {
    // config
    std::string phyMode("OfdmRate6Mbps");
    m_ssid = Ssid("ns-3-ssid");
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
    m_wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode) /*, "ControlMode", StringValue(phyMode)*/);

    // energy
    m_basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(25.0));
    m_radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));

    // ip
    m_ipv4.SetBase(network, mask);
  }

  Ptr<IoTNetNode> IoTNetWifi::Create(std::string id, Vector position)
  {
    u_int32_t currentIndex = m_allNodes.GetN();

    Ptr<IoTNetNode> pack = CreateObject<IoTNetNode>();
    pack->id = id;
    pack->position = position;

    // create node
    pack->node.Create(1);
    m_allNodes.Add(pack->node);

    // device = phy + mac + node
    if (currentIndex == 0) // node 0 always ap
    {
      m_wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(m_ssid));
    }
    else
    {
      m_wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(m_ssid), "ActiveProbing", BooleanValue(false));
    }
    pack->device = m_wifi.Install(m_wifiPhy, m_wifiMac, pack->node);

    // energy
    EnergySourceContainer energySource = m_basicSourceHelper.Install(pack->node);
    DeviceEnergyModelContainer deviceEnergyModel = m_radioEnergyHelper.Install(pack->device, energySource);

    // add to world
    IoTNet::world->Add(id, pack->node, position);

    // ip
    pack->interface = m_ipv4.Assign(pack->device);

    // append to vector
    m_allPacks.push_back(pack);

    return pack;
  }
}