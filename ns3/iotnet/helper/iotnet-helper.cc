#include "iotnet-helper.h"

#include "ns3/names.h"
#include "ns3/uinteger.h"

namespace ns3
{

// local
static void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND("Cwnd change at " << Simulator::Now().GetSeconds() << " with value " << newCwnd);
}

static void
OnDataReceived(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  while ((packet = socket->Recv())) {
    // Process the received data
    uint8_t buffer[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    
    // Assuming the payload is a string, you can convert it to a C++ string
    std::string payload(reinterpret_cast<char*>(buffer), packet->GetSize());

    NS_LOG_UNCOND("Received " << packet->GetSize() << " bytes. Payload: " << payload);
  }
}

static void
OnConnectionAccepted(Ptr<Socket> socket, const Address &address) {
    NS_LOG_UNCOND("Received connection from " << address);

    socket->SetRecvCallback(MakeCallback(&OnDataReceived));
}

/*** IOTNET_SENSOR_HELPER ***/

// public
IoTNetSensorHelper::IoTNetSensorHelper(Address address)
{
    m_factory.SetTypeId(IoTNetSensor::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
}

void
IoTNetSensorHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
IoTNetSensorHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

// private
Ptr<Application>
IoTNetSensorHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Socket> socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
    socket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

    Ptr<IoTNetSensor> app = m_factory.Create<IoTNetSensor>();
    app->SetSocket(socket);
    node->AddApplication(app);

    return app;
}

/*** IOTNET_SERVER_HELPER ***/

// public
IoTNetServerHelper::IoTNetServerHelper(Address address)
{
    m_factory.SetTypeId(IoTNetServer::GetTypeId());
    SetAttribute("Address", AddressValue(address));
}

void
IoTNetServerHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
IoTNetServerHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

// private
Ptr<Application>
IoTNetServerHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Socket> socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
    socket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

    Ptr<IoTNetServer> app = m_factory.Create<IoTNetServer>();
    app->SetSocket(socket);
    node->AddApplication(app);

    socket->SetAcceptCallback(
      MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
      MakeCallback(&OnConnectionAccepted));

    return app;
}

}
