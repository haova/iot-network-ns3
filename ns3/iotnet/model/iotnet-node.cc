#include "iotnet-node.h"

namespace ns3
{
  std::string IoTNetNode::LogPrefix()
  {
    std::stringstream ss;
    ss << "+ [" << id << "](" << Simulator::Now().GetSeconds() << "s): ";
    return ss.str();
  }

  void IoTNetNode::RemainingEnergy(double oldValue, double remainingEnergy)
  {
    NS_LOG_UNCOND(LogPrefix() << "Current remaining energy = " << remainingEnergy << "J");
  }

  void IoTNetNode::TotalEnergy(double oldValue, double totalEnergy)
  {
    NS_LOG_UNCOND(LogPrefix() << "Total energy consumed by radio = " << totalEnergy << "J");
  }
}