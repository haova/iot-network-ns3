/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IOTNET_WIFI_H
#define IOTNET_WIFI_H

#include "ns3/core-module.h"
#include "ns3/energy-module.h"
#include "ns3/internet-module.h"
#include "ns3/jamming-module.h"
#include "ns3/mobility-module.h"
#include "ns3/node-container.h"
#include "ns3/wifi-module.h"

namespace ns3
{
  class IoTNetNodePack : public Object
  {
  public:
    std::string id;
    Vector position;
    NodeContainer node;
    NetDeviceContainer device;
  };

  class IoTNetWifi
  {
  public:
    IoTNetWifi(const std::string id, const InternetStackHelper internet, const Ipv4Address network, const Ipv4Mask mask);
    Ptr<IoTNetNodePack> Create(std::string id, Vector position);
    void Install();

  private:
    std::string m_id;
    NodeContainer m_allNodes;

    MobilityHelper m_mobility;
    Ptr<ListPositionAllocator> m_positionAlloc;

    WifiHelper m_wifi;
    NslWifiPhyHelper m_wifiPhy;
    NslWifiChannelHelper m_wifiChannel;
    WifiMacHelper m_wifiMac;

    BasicEnergySourceHelper m_basicSourceHelper;
    WifiRadioEnergyModelHelper m_radioEnergyHelper;

    InternetStackHelper m_internet;
    Ipv4AddressHelper m_ipv4;

    std::vector<Ptr<IoTNetNodePack>> m_allPacks;
  };
}
#endif /* IOTNET_WIFI_H */