/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IOTNET_H
#define IOTNET_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

namespace ns3
{
  class IoTNet : public Object
  {
  public:
    static Ptr<IoTNet> world;
    IoTNet();
    void Add(const std::string name, NodeContainer nodes, Vector position);
    void Add(const std::string name, NodeContainer nodes, Vector position, std::string icon);
    void Install();
    void UpdateAnimationInterface(AnimationInterface anim);

  private:
    InternetStackHelper m_internet;
    MobilityHelper m_mobility;
    Ptr<ListPositionAllocator> m_positionAlloc;
    NodeContainer m_allNodes;
    std::vector<std::string> m_allNames;
    std::vector<std::string> m_allIcons;
  };
}

#endif /* IOTNET_H */
