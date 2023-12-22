/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IOTNET_NODE_H
#define IOTNET_NODE_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/wifi-module.h"
#include "ns3/ipv4-interface-container.h"

namespace ns3
{
  class IoTNetNode : public Object
  {
  public:
    std::string id;
    Vector position;
    NodeContainer node;
    NetDeviceContainer device;
    Ipv4InterfaceContainer interface;

    std::string LogPrefix();
    void RemainingEnergy(double oldValue, double remainingEnergy);
    void TotalEnergy(double oldValue, double totalEnergy);
  };
}
#endif /* IOTNET_NODE_H */