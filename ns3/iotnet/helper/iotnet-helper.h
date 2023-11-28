#ifndef IOTNET_HELPER_H
#define IOTNET_HELPER_H

#include "ns3/iotnet-sensor.h"
#include "ns3/iotnet-server.h"
#include "ns3/iotnet.h"

#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

namespace ns3
{

class IoTNetSensorHelper
{
  public:
    IoTNetSensorHelper(Address ip, uint16_t port);
    void SetAttribute(std::string name, const AttributeValue& value);
    ApplicationContainer Install(Ptr<Node> node) const;

  private:
    Ptr<Application> InstallPriv(Ptr<Node> node) const;
    
    ObjectFactory m_factory;
};

} // namespace ns3

#endif /* IOTNET_HELPER_H */
