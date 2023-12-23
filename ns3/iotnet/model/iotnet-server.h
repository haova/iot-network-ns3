/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IOTNET_SERVER_H
#define IOTNET_SERVER_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/point-to-point-module.h"

namespace ns3
{
  class IoTNetServer : public Object
  {
  public:
    IoTNetServer(const std::string id, const Ipv4Address network, const Ipv4Mask mask, Vector position);
    void Add(const NodeContainer nodes);
    void ConnectionAcceptedCallback(Ptr<Socket> socket, const Address &address);
    void DataReceivedCallback(Ptr<Socket> socket);
    Address GetAddress();

  private:
    NodeContainer m_node;
    Ipv4AddressHelper m_ipv4;
    PointToPointHelper p2p;
    Address m_sinkAddress;
  };
}

#endif /* IOTNET_SERVER_H */
