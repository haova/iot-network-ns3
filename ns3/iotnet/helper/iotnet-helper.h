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
    IoTNetSensorHelper(Address address);
    void SetAttribute(std::string name, const AttributeValue &value);
    ApplicationContainer Install(Ptr<NetDevice> device) const;
    ApplicationContainer Install(std::string name, Ptr<NetDevice> device) const;

  private:
    Ptr<Application> InstallPriv(std::string name, Ptr<NetDevice> device) const;

    ObjectFactory m_factory;
  };

  class IoTNetServerHelper
  {
  public:
    IoTNetServerHelper(Address address);
    void SetAttribute(std::string name, const AttributeValue &value);
    ApplicationContainer Install(Ptr<NetDevice> device) const;
    ApplicationContainer Install(std::string name, Ptr<NetDevice> device) const;

  private:
    Ptr<Application> InstallPriv(std::string name, Ptr<NetDevice> device) const;

    ObjectFactory m_factory;
  };

} // namespace ns3

#endif /* IOTNET_HELPER_H */
