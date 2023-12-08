#ifndef IOTNET_SENSOR_H
#define IOTNET_SENSOR_H

#include "iotnet-app.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-helper.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace ns3
{
  class IoTNetSensor : public IoTNetApp
  {
  public:
    IoTNetSensor();
    ~IoTNetSensor() override;

    static TypeId GetTypeId();
    void SetSocket(Ptr<Socket> socket);
    void SendPacket(std::string message);
    void ReceiveOkCallback(Ptr<const WifiPsdu> psdu,
                           RxSignalInfo rxSignalInfo,
                           WifiTxVector txVector,
                           std::vector<bool> statusPerMpdu);

  private:
    void AfterStart() override;
    void BeforeStop() override;

    void Main();
    void Loop();

    Address m_peerAddress;
    Ptr<Socket> m_socket;
    json m_payload;
  };

} // namespace ns3

#endif /* IOTNET_SENSOR_H */
