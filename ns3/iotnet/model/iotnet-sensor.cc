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

    void IoTNetSensor::Main()
    {
    }

    void IoTNetSensor::Loop()
    {
        if (!m_running)
            return;

        std::cout << LogPrefix() << "Preparing data" << std::endl;

        std::stringstream ss;
        ss << "{";
        ss << "\"name\":\"" << m_name << "\", ";
        ss << "\"at\":" << Now() * 1000 << ", ";
        ss << "\"rssi\": [";

        for (auto it = m_rssi.begin(); it != m_rssi.end(); it++)
        {
            if (it != m_rssi.begin())
            {
                ss << ", ";
            }
            ss << *it;
        }
        ss << "]";

        ss << "}";

        m_rssi.clear();

        SendPacket(ss.str());

        m_scheduleEvent = Simulator::Schedule(Seconds(1), &IoTNetSensor::Loop, this);
    }

    void
    IoTNetSensor::AfterStart()
    {
        m_socket->Bind();
        m_socket->Connect(m_peerAddress);

        Main();
        Loop();
    }

    void
    IoTNetSensor::BeforeStop()
    {
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
        m_rssi.push_back(rxSignalInfo.rssi);
        std::cout << LogPrefix() << rxSignalInfo << std::endl;
    }

} // namespace ns3
