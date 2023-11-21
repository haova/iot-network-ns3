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
                            .AddConstructor<IoTSensor>();
    return tid;
}

void
IoTSensor::StartApplication()
{
}

void
IoTSensor::StopApplication()
{
}

} // namespace ns3