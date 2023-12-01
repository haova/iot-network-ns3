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
                            .AddConstructor<IoTNetServer>()
                            .AddAttribute("Address",
                                          "Server address to serve",
                                          AddressValue(),
                                          MakeAddressAccessor(&IoTNetServer::m_address),
                                          MakeAddressChecker());
    return tid;
}

void
IoTNetServer::StartApplication()
{
    std::cout << "Start server app" << std::endl;

    m_socket->Bind(m_address);
    m_socket->Listen();
}

void
IoTNetServer::StopApplication()
{
    std::cout << "Stop server app" << std::endl;
}

void
IoTNetServer::SetSocket(Ptr<Socket> socket)
{
    m_socket = socket;
}

} // namespace ns3
