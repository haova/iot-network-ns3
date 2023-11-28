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

// public
IoTNetSensorHelper::IoTNetSensorHelper(Address address, uint16_t port)
{
    m_factory.SetTypeId(IoTNetSensor::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
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


}
