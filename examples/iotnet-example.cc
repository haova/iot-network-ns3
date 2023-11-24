/**
 *  Sensor         AP
 *  *              *
 *  |   10.1.3.0   |    10.1.1.0
 * n2      ))     n0 -------------- n1
 *      wireless     point-to-point  |
 *                                   Server
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/iotnet-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IoTNetworkSimulator");

void
RssiCallback(std::string context,
             Ptr<const Packet> p,
             double snr,
             WifiMode mode,
             WifiPreamble preamble)
{
    double rssi = 10 * std::log10(snr);
    std::cout << "Received packet with RSSI: " << rssi << " dBm" << std::endl;
}

int
main(int argc, char* argv[])
{
    NS_LOG_UNCOND("IoT Network Simulator");

    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // create p2p nodes (n0, n1)
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    // create wifi sensor nodes (n2)
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(1);

    // get AP node (n0)
    NodeContainer wifiApNode = p2pNodes.Get(0);

    // get server node (n1)
    NodeContainer serverNode;
    serverNode.Add(p2pNodes.Get(1));

    // wifi phy and mac
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");

    WifiHelper wifi;

    // p2p devices
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    // wireless devices
    NetDeviceContainer staDevices;
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, wifiStaNodes);

    NetDeviceContainer apDevices;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevices = wifi.Install(phy, mac, wifiApNode);

    // mobility
    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(wifiStaNodes);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);

    // internet ipv4 stack
    InternetStackHelper stack;
    stack.Install(p2pNodes);
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);

    // assign ip addresses
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("10.1.3.0", "255.255.255.0");
    address.Assign(staDevices);
    address.Assign(apDevices);

    // server app
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(serverNode.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // client
    // IoTSensorHelper iotSensorHelper(p2pInterfaces.GetAddress(1), 9);
    // ApplicationContainer sensorApp = iotSensorHelper.Install(wifiStaNodes.Get(0));
    // sensorApp.Start(Seconds(2.0));
    // sensorApp.Stop(Seconds(10.0));

    // UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(1), 9);
    // echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    // echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    // echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    // ApplicationContainer clientApp = echoClient.Install(wifiStaNodes.Get(0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // tracing
    Config::Connect("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/State/RxOk",
                    MakeCallback(&RssiCallback));

    // net animation
    // AnimationInterface anim("wireless-animation.xml");
    // anim.EnablePacketMetadata();
    // anim.UpdateNodeDescription(p2pNodes.Get(0), "AP (n0)");
    // anim.UpdateNodeDescription(p2pNodes.Get(1), "Server (n1)");
    // anim.UpdateNodeDescription(wifiStaNodes.Get(0), "Sensor 1 (n2)");

    // pcap tracing
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    pointToPoint.EnablePcapAll("net");
    phy.EnablePcap("net", apDevices.Get(0));

    // run simulation
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}