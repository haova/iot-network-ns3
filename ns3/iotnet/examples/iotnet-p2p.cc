#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/iotnet-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IoTNetworkSimulator");

/**
 * Congestion window change callback
 *
 * \param oldCwnd Old congestion window.
 * \param newCwnd New congestion window.
 */
static void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND("test packet " << Simulator::Now().GetSeconds() << "\t" << newCwnd);
}

/**
 * Rx drop callback
 *
 * \param p The dropped packet.
 */
static void
RxDrop(Ptr<const Packet> p)
{
    NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds());
}

int
main(int argc, char* argv[])
{
    NS_LOG_UNCOND("IoT Network Simulator");

    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                       TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));

    // create nodes
    NodeContainer nodes;
    nodes.Create(2);

    // create edge, physic connector
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("200ms"));

    // install node to connector, display as device
    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // pcap and ascii tracing
    InternetStackHelper stack;
    stack.Install(nodes);

    // assign IPs
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    // ipv4
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // sink app
    uint16_t sinkPort = 8080;
    Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), sinkPort));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                      InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get(1));
    sinkApps.Start(Seconds(0.));
    sinkApps.Stop(Seconds(20.));

    // Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());
    // ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

    // sensor app
    IoTNetSensorHelper iotNetSensorHelper(sinkAddress, sinkPort);
    ApplicationContainer app = iotNetSensorHelper.Install(nodes.Get(0));
    app.Start(Seconds(1.0));
    app.Stop(Seconds(20.0));

    // Ptr<IoTNetSensor> app = CreateObject<IoTNetSensor>();
    // app->SetAttribute("RemoteAddress", AddressValue(sinkAddress));
    // app->SetSocket(ns3TcpSocket);
    // nodes.Get(0)->AddApplication(app);
    // app->SetStartTime(Seconds(1.));
    // app->SetStopTime(Seconds(20.));

    devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));

    // net animation
    AnimationInterface anim("output/iot.xml");
    anim.SetConstantPosition(nodes.Get(0), 10.0, 10.0);
    anim.SetConstantPosition(nodes.Get(1), 20.0, 20.0);

    // ascii format tracing
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("output/iot.tr"));
    pointToPoint.EnablePcapAll("output/iot");

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
