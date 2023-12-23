/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IOTNET_WIFI_H
#define IOTNET_WIFI_H

#include "ns3/core-module.h"
#include "ns3/energy-module.h"
#include "ns3/internet-module.h"
#include "ns3/jamming-module.h"
#include "ns3/node-container.h"
#include "ns3/wifi-module.h"

#include "ns3/iotnet.h"
#include "ns3/iotnet-node.h"

#include <ctime>

namespace ns3
{
  class IoTNetWifi
  {
  public:
    static int currentWifiId;

    IoTNetWifi(const std::string id, const Ipv4Address network, const Ipv4Mask mask, const Vector position);
    Ptr<IoTNetNode> Create(std::string id, Vector position);
    Ptr<IoTNetNode> GetAp();
    void Install();
    void GatherInformation();
    void Loop();
    std::time_t Now();

    double DbmToW(double dBm) const;
    double WToDbm(double w) const;

  private:
    std::string m_id;
    Ssid m_ssid;
    NodeContainer m_allNodes;

    WifiHelper m_wifi;
    NslWifiPhyHelper m_wifiPhy;
    NslWifiChannelHelper m_wifiChannel;
    WifiMacHelper m_wifiMac;

    BasicEnergySourceHelper m_basicSourceHelper;
    WifiRadioEnergyModelHelper m_radioEnergyHelper;

    Ipv4AddressHelper m_ipv4;

    std::vector<Ptr<IoTNetNode>> m_allIoTNode;
  };
}
#endif /* IOTNET_WIFI_H */