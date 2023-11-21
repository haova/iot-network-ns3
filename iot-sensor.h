#ifndef IOT_SENSOR_H
#define IOT_SENSOR_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

namespace ns3
{
class IoTSensor : public Application
{
  public:
    IoTSensor();
    ~IoTSensor() override;

    static TypeId GetTypeId();

  private:
    void StartApplication() override;
    void StopApplication() override;

    Address m_peerAddress; //!< Remote peer address
    uint16_t m_peerPort;   //!< Remote peer port
};

} // namespace ns3

#endif /* IOT_SENSOR_H */
