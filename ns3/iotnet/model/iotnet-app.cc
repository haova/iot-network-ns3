#include "iotnet-app.h"

#include <ctime>
#include <iostream>

namespace ns3
{
  void
  IoTNetApp::AfterStart()
  {
  }

  void
  IoTNetApp::BeforeStop()
  {
  }

  void
  IoTNetApp::StartApplication()
  {
    std::cout << LogPrefix() << "Starting sensor app" << std::endl;
    m_running = true;

    AfterStart();
  }

  void
  IoTNetApp::StopApplication()
  {
    std::cout << LogPrefix() << "Stopping sensor app" << std::endl;
    BeforeStop();

    m_running = false;
    if (m_scheduleEvent.IsRunning())
    {
      Simulator::Cancel(m_scheduleEvent);
    }
  }

  void IoTNetApp::CwndChangeCallback(uint32_t oldCwnd, uint32_t newCwnd)
  {
    std::cout << LogPrefix() << "Cwnd change at " << Simulator::Now().GetSeconds() << " with value " << newCwnd << std::endl;
  }

  void IoTNetApp::SetName(std::string name)
  {
    m_name = name;
  }

  std::time_t IoTNetApp::Now()
  {
    return std::time(nullptr);
  }

  std::string IoTNetApp::FormatDate(std::time_t seconds)
  {
    char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
    std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", std::gmtime(&seconds));
    return timeString;
  }

  std::string IoTNetApp::LogPrefix()
  {

    return "[" + FormatDate(Now()) + "] (" + m_name + ") ";
  }

} // namespace ns3
