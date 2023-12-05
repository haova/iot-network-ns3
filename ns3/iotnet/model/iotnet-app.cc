#include "iotnet-app.h"

#include <iostream>

namespace ns3
{
  void IoTNetApp::CwndChangeCallback(uint32_t oldCwnd, uint32_t newCwnd)
  {
    std::cout << LogPrefix() << "Cwnd change at " << Simulator::Now().GetSeconds() << " with value " << newCwnd << std::endl;
  }

  void IoTNetApp::SetName(std::string name)
  {
    m_name = name;
  }

  std::string IoTNetApp::LogPrefix()
  {
    return "[" + m_name + "] ";
  }

} // namespace ns3
