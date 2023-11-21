#include "iot-sensor.h"

namespace ns3
{

IoTSensor::IoTSensor()
{
}

IoTSensor::~IoTSensor()
{
}

TypeId
IoTSensor::GetTypeId()
{
    static TypeId tid = TypeId("IoTSensor")
                            .SetParent<Application>()
                            .SetGroupName("IoT")
                            .AddConstructor<IoTSensor>()
                            .AddAttribute("RemoteAddress",
                                          "The destination Address of the outbound packets",
                                          AddressValue(),
                                          MakeAddressAccessor(&IoTSensor::m_peerAddress),
                                          MakeAddressChecker())
                            .AddAttribute("RemotePort",
                                          "The destination port of the outbound packets",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&IoTSensor::m_peerPort),
                                          MakeUintegerChecker<uint16_t>());
    return tid;
}

void
IoTSensor::StartApplication()
{
    std::cout << "Start sensor app" << std::endl;
}

void
IoTSensor::StopApplication()
{
    std::cout << "Stop sensor app" << std::endl;
}

} // namespace ns3