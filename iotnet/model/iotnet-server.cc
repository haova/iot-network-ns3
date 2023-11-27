#include "iotnet-server.h"

#include <iostream>

namespace ns3
{

IoTNetServer::IoTNetServer()
{
}

IoTNetServer::~IoTNetServer()
{
}

TypeId
IoTNetServer::GetTypeId()
{
    static TypeId tid = TypeId("IoTNetServer")
                            .SetParent<Application>()
                            .SetGroupName("IoTNet")
                            .AddConstructor<IoTNetServer>();
    return tid;
}

void
IoTNetServer::StartApplication()
{
    std::cout << "Start server app" << std::endl;
}

void
IoTNetServer::StopApplication()
{
    std::cout << "Stop server app" << std::endl;
}

} // namespace ns3
