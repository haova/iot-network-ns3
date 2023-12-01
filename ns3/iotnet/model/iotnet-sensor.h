#ifndef IOTNET_SENSOR_H
#define IOTNET_SENSOR_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

// Add a doxygen group for this module.
// If you have more than one file, this should be in only one of them.
/**
 * \defgroup iotnet Description of the iotnet
 */

namespace ns3
{

// Each class should be documented using Doxygen,
// and have an \ingroup iotnet directive

class IoTNetSensor : public Application
{
  public:
    IoTNetSensor();
    ~IoTNetSensor() override;

    static TypeId GetTypeId();
    void SetSocket(Ptr<Socket> socket);
    void SendPacket(std::string message);

  private:
    void StartApplication() override;
    void StopApplication() override;

    Address m_peerAddress;
    Ptr<Socket> m_socket;
};

} // namespace ns3

#endif /* IOTNET_SENSOR_H */
