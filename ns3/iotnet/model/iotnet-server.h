/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IOTNET_SERVER_H
#define IOTNET_SERVER_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/csma-module.h"

namespace ns3
{
  class IoTNetServer : public Object
  {
  public:
    IoTNetServer(const std::string id, const Ipv4Address network, const Ipv4Mask mask, Vector position);
    void Add(const NodeContainer nodes);

  private:
    Ipv4AddressHelper m_ipv4;
    CsmaHelper m_csma;
  };
}

#endif /* IOTNET_SERVER_H */
