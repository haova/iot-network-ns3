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

    // p2p
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // ipv4
    m_ipv4.SetBase(network, mask);

    // internet
    IoTNet::world->Add(id, m_node, position);
  }

  void IoTNetRouter::Add(NodeContainer nodes)
  {
    for (size_t i = 0; i < nodes.GetN(); i++)
    {
      NodeContainer group(m_node, nodes.Get(i));
      NetDeviceContainer devices = p2p.Install(group);

      std::ostringstream subnet;
      subnet << "10.2." << i + 1 << ".0";
      m_ipv4.SetBase(subnet.str().c_str(), "255.255.255.0");
      Ipv4InterfaceContainer interfaces = m_ipv4.Assign(devices);

      NS_LOG_UNCOND("Router ip " << interfaces.GetAddress(0));
      NS_LOG_UNCOND("Next hop ip " << interfaces.GetAddress(1));
    }
  }

  NodeContainer IoTNetRouter::GetNode()
  {
    return m_node;
  }
}
