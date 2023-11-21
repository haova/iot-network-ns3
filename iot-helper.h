#ifndef IOT_HELPER_H
#define IOT_HELPER_H

#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/object-factory.h"

namespace ns3
{

class IoTSensor : public Application
{
  public:
    static TypeId GetTypeId();
    IoTSensor();
};

class IoTSensorHelper
{
  public:
    IoTSensorHelper(Address ip, uint16_t port);
    void SetAttribute(std::string name, const AttributeValue& value);

  private:
    ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* IOT_HELPER_H */
