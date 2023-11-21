#include "iot-helper.h"

#include "ns3/names.h"
#include "ns3/uinteger.h"

namespace ns3
{

IoTSensorHelper::IoTSensorHelper(Address address, uint16_t port)
{
    m_factory.SetTypeId("ns3::IoTSensor");
    SetAttribute("RemoteAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
}

void
IoTSensorHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

} // namespace ns3