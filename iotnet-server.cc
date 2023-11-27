#include "iotnet-server.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/stats-module.h"
#include "cpr/cpr.h"


#include <ostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("IotNetServer");

TypeId
IotNetServer::GetTypeId()
{
    static TypeId tid = TypeId("IotNetServer")
                            .SetParent<Application>()
                            .AddConstructor<IotNetServer>()
                            .AddAttribute("Port",
                                          "Listening port.",
                                          UintegerValue(1603),
                                          MakeUintegerAccessor(&IotNetServer::m_port),
                                          MakeUintegerChecker<uint32_t>());
    return tid;
}

IotNetServer::IotNetServer()
{
    NS_LOG_FUNCTION_NOARGS();
}

IotNetServer::~IotNetServer()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
IotNetServer::DoDispose()
{
    NS_LOG_FUNCTION_NOARGS();

    m_socket = nullptr;
    // chain up
    Application::DoDispose();
}

void
IotNetServer::StartApplication()
{
    NS_LOG_FUNCTION_NOARGS();

    // if (!m_socket)
    // {
    //     Ptr<SocketFactory> socketFactory =
    //         GetNode()->GetObject<SocketFactory>(UdpSocketFactory::GetTypeId());
    //     m_socket = socketFactory->CreateSocket();
    //     InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
    //     m_socket->Bind(local);
    // }

    // m_socket->SetRecvCallback(MakeCallback(&IotNetServer::Receive, this));
}

void
IotNetServer::StopApplication()
{
    NS_LOG_FUNCTION_NOARGS();

    // if (m_socket)
    // {
    //     m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    // }
}

void
IotNetServer::SetCounter(Ptr<CounterCalculator<>> calc)
{
    m_calc = calc;
}

void
IotNetServer::SetDelayTracker(Ptr<TimeMinMaxAvgTotalCalculator> delay)
{
    m_delay = delay;
}

void
IotNetServer::Receive(Ptr<Socket> socket)
{
    // NS_LOG_FUNCTION (this << socket << packet << from);

    // Ptr<Packet> packet;
    // Address from;
    // while ((packet = socket->RecvFrom(from)))
    // {
    //     if (InetSocketAddress::IsMatchingType(from))
    //     {
    //         NS_LOG_INFO("Received " << packet->GetSize() << " bytes from "
    //                                 << InetSocketAddress::ConvertFrom(from).GetIpv4());

    //         cpr::Response r = cpr::Post(cpr::Url{"testpage.com"},
    //                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
    //                            cpr::Parameters{{"anon", "true"}, {"key", "value"}});

    //         NS_LOG_INFO(r.text);
    //     }

    //     TimestampTag timestamp;
    //     // Should never not be found since the sender is adding it, but
    //     // you never know.
    //     if (packet->FindFirstMatchingByteTag(timestamp))
    //     {
    //         Time tx = timestamp.GetTimestamp();

    //         if (m_delay)
    //         {
    //             m_delay->Update(Simulator::Now() - tx);
    //         }
    //     }

    //     if (m_calc)
    //     {
    //         m_calc->Update();
    //     }
    // }
}
