/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IOTNET_ROUTER_H
#define IOTNET_ROUTER_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/point-to-point-module.h"

namespace ns3
{
  class IoTNetRouter : public Object
  {
  public:
    IoTNetRouter(const std::string id, const Ipv4Address network, const Ipv4Mask mask, Vector position);
    void Add(const NodeContainer nodes);
    NodeContainer GetNode();

  private:
    NodeContainer m_node;
    Ipv4AddressHelper m_ipv4;
    PointToPointHelper p2p;
  };
}

#endif /* IOTNET_ROUTER_H */
