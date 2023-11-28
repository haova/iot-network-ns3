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
                                          MakeAddressChecker())
                            .AddAttribute("RemotePort",
                                          "The destination port of the outbound packets",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&IoTNetSensor::m_peerPort),
                                          MakeUintegerChecker<uint16_t>());
    return tid;
}

void
IoTNetSensor::StartApplication()
{
    std::cout << "Start sensor app" << std::endl;

    m_socket->Bind();
    m_socket->Connect(m_peerAddress);

    SendPacket("Hello, World!");
}

void
IoTNetSensor::StopApplication()
{
    std::cout << "Stop sensor app" << std::endl;
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

} // namespace ns3
