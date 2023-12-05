#ifndef IOTNET_APP_H
#define IOTNET_APP_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-helper.h"

namespace ns3
{
  class IoTNetApp : public Application
  {
  public:
    void CwndChangeCallback(uint32_t oldCwnd, uint32_t newCwnd);
    void SetName(std::string name);
    std::string LogPrefix();

  private:
    std::string m_name;
  };

} // namespace ns3

#endif /* IOTNET_APP_H */
