#include "iot-helper.h"

#include "ns3/names.h"
#include "ns3/uinteger.h"

namespace ns3
{

IoTSensorHelper::IoTSensorHelper(Address address, uint16_t port)
{
    m_factory.SetTypeId(IoTSensor::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
}

void
IoTSensorHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
IoTSensorHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

Ptr<Application>
IoTSensorHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<IoTSensor>();
    node->AddApplication(app);

    return app;
}

} // namespace ns3