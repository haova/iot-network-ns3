/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/node-container.h"
#include "ns3/iotnet.h"

#include "iotnet-router.h"

namespace ns3
{
  IoTNetRouter::IoTNetRouter(const std::string id, const Ipv4Address network, const Ipv4Mask mask, Vector position)
  {
    // create node
    m_node.Create(1);

    // csma
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // ipv4
    m_ipv4.SetBase(network, mask);

    // internet
    IoTNet::world->Add(id, m_node, position);
  }

  void IoTNetRouter::Add(NodeContainer node)
  {
    NetDeviceContainer devices = p2p.Install(NodeContainer(m_node, node));
    Ipv4InterfaceContainer interfaces = m_ipv4.Assign(devices);

    NS_LOG_UNCOND("Router ip " << interfaces.GetAddress(0));
    NS_LOG_UNCOND("Next hop ip " << interfaces.GetAddress(1));
  }

  NodeContainer IoTNetRouter::GetNode()
  {
    return m_node;
  }
}
