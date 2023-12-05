#include "iotnet-server.h"

#include <iostream>
#include "cpr/cpr.h"

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
        std::cout << LogPrefix() << "Start server app" << std::endl;

        m_socket->Bind(m_address);
        m_socket->Listen();
    }

    void
    IoTNetServer::StopApplication()
    {
        std::cout << LogPrefix() << "Stop server app" << std::endl;
    }

    void
    IoTNetServer::SetSocket(Ptr<Socket> socket)
    {
        m_socket = socket;
    }

    void
    IoTNetServer::DataReceivedCallback(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        while ((packet = socket->Recv()))
        {
            packet->PrintPacketTags(std::cout);

            // Process the received data
            uint8_t buffer[packet->GetSize()];
            packet->CopyData(buffer, packet->GetSize());

            // Assuming the payload is a string, you can convert it to a C++ string
            std::string payload(reinterpret_cast<char *>(buffer), packet->GetSize());

            std::cout << LogPrefix() << "Received " << packet->GetSize() << " bytes. Payload: " << payload << std::endl;

            // forward to external server
            cpr::Response r = cpr::Post(
                cpr::Url{"server-node:8080"},
                cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                cpr::Parameters{{"anon", "true"}, {"key", "value"}},
                cpr::Body{payload});
        }
    }

    void IoTNetServer::ConnectionAcceptedCallback(Ptr<Socket> socket, const Address &address)
    {
        std::cout << LogPrefix() << "Received connection from " << address << std::endl;
        socket->SetRecvCallback(MakeCallback(&IoTNetServer::DataReceivedCallback, this));
    }

} // namespace ns3
