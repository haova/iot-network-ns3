#ifndef IOTNET_SERVER_H
#define IOTNET_SERVER_H

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

class IoTNetServer : public Application
{
  public:
    IoTNetServer();
    ~IoTNetServer() override;

    static TypeId GetTypeId();

  private:
    void StartApplication() override;
    void StopApplication() override;
};

} // namespace ns3

#endif /* IOTNET_SERVER_H */
