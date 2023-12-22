/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "iotnet.h"

namespace ns3
{
  Ptr<IoTNet> IoTNet::world = nullptr;

  IoTNet::IoTNet()
  {
    m_positionAlloc = CreateObject<ListPositionAllocator>();
  }

  void IoTNet::Add(const std::string name, NodeContainer nodes, Vector position)
  {
    m_internet.Install(nodes);

    for (size_t i = 0; i < nodes.GetN(); i++)
    {
      Ptr<Node> n0 = nodes.Get(i);
      m_allNodes.Add(n0);
      m_positionAlloc->Add(position);
      m_allNames.push_back(name);
    }
  }

  void IoTNet::Install()
  {
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    m_mobility.SetPositionAllocator(m_positionAlloc);
    m_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    m_mobility.Install(m_allNodes);
  }

  void IoTNet::UpdateAnimationInterface(AnimationInterface anim)
  {
    for (size_t i = 0; i < m_allNodes.GetN(); i++)
    {
      anim.UpdateNodeDescription(m_allNodes.Get(i), m_allNames.at(i));
    }
  }
}
