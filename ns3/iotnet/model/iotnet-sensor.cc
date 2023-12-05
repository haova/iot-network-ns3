#include "iotnet-sensor.h"

#include <iostream>

namespace ns3
{

    IoTNetSensor::IoTNetSensor()
    {
    }

    IoTNetSensor::~IoTNetSensor()
    {
    }

    TypeId
    IoTNetSensor::GetTypeId()
    {
        static TypeId tid = TypeId("IoTNetSensor")
                                .SetParent<Application>()
                                .SetGroupName("IoTNet")
                                .AddConstructor<IoTNetSensor>()
                                .AddAttribute("RemoteAddress",
                                              "The destination Address of the outbound packets",
                                              AddressValue(),
                                              MakeAddressAccessor(&IoTNetSensor::m_peerAddress),
                                              MakeAddressChecker());
        return tid;
    }

    void
    IoTNetSensor::StartApplication()
    {
        std::cout << LogPrefix() << "Start sensor app" << std::endl;

        m_socket->Bind();
        m_socket->Connect(m_peerAddress);

        SendPacket("Hello, World!");
    }

    void
    IoTNetSensor::StopApplication()
    {
        std::cout << LogPrefix() << "Stop sensor app" << std::endl;
    }

    void
    IoTNetSensor::SetSocket(Ptr<Socket> socket)
    {
        m_socket = socket;
    }

    void
    IoTNetSensor::SendPacket(std::string message)
    {
        Ptr<Packet> packet = Create<Packet>((uint8_t *)message.c_str(), message.length());
        m_socket->Send(packet);
    }

    void IoTNetSensor::ReceiveOkCallback(Ptr<const WifiPsdu> psdu,
                                         RxSignalInfo rxSignalInfo,
                                         WifiTxVector txVector,
                                         std::vector<bool> statusPerMpdu)
    {
        std::cout << LogPrefix() << rxSignalInfo << std::endl;
    }

} // namespace ns3
