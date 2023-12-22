/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/node-container.h"
#include "ns3/iotnet.h"

#include "iotnet-server.h"

namespace ns3
{
  IoTNetServer::IoTNetServer(const std::string id, const Ipv4Address network, const Ipv4Mask mask, Vector position)
  {
    // create node
    NodeContainer node;
    node.Create(1);

    // csma
    m_csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    m_csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    // ipv4
    m_ipv4.SetBase(network, mask);

    // internet
    IoTNet::world->Add(id, node, position);

    // create device
    NetDeviceContainer device = m_csma.Install(node);
    Ipv4InterfaceContainer interface = m_ipv4.Assign(device);

    NS_LOG_UNCOND("Server ip " << interface.GetAddress(0));
  }

  void IoTNetServer::Add(NodeContainer nodes)
  {
    NetDeviceContainer devices = m_csma.Install(nodes);
    Ipv4InterfaceContainer interfaces = m_ipv4.Assign(devices);

    NS_LOG_UNCOND("Ap ip " << interfaces.GetAddress(0));
  }
}
