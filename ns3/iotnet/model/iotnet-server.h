#ifndef IOTNET_SERVER_H
#define IOTNET_SERVER_H

#include "iotnet-app.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

namespace ns3
{
  class IoTNetServer : public IoTNetApp
  {
  public:
    IoTNetServer();
    ~IoTNetServer() override;

    static TypeId GetTypeId();
    void SetSocket(Ptr<Socket> socket);
    void ConnectionAcceptedCallback(Ptr<Socket> socket, const Address &address);
    void DataReceivedCallback(Ptr<Socket> socket);

  private:
    void StartApplication() override;
    void StopApplication() override;

    Address m_address;
    Ptr<Socket> m_socket;
  };

} // namespace ns3

#endif /* IOTNET_SERVER_H */
