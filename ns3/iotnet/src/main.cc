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
#include "ns3/energy-module.h"
#include "ns3/jammer-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IoTNetworkSimulator");

void NodeRss(double oldValue, double rss)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s Node RSS = " << rss << "W");
}

void NodePdr(double oldValue, double pdr)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s Node PDR = " << pdr);
}

void NodeThroughputRx(double oldValue, double rxThroughput)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s Node RX throughput = "
                                                << rxThroughput);
}

NodeContainer
createSensorNodes(int n)
{
    NodeContainer container;
    container.Create(n);
    return container;
}

NetDeviceContainer
setupWifi(NodeContainer sensorNodes, NodeContainer apNode)
{
    // nodes
    int jammerIndex = sensorNodes.GetN() - 1;
    NodeContainer honestNodes;
    for (int i = 0; i < jammerIndex; i++)
        honestNodes.Add(sensorNodes.Get(i));

    NodeContainer jammerNodes;
    jammerNodes.Add(sensorNodes.Get(jammerIndex));

    // wifi phy and mac
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");

    WifiHelper wifi;

    // wireless devices
    NetDeviceContainer staDevices;
    NetDeviceContainer jammerNetdevice;
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, honestNodes);
    jammerNetdevice = wifi.Install(phy, mac, jammerNodes);

    NetDeviceContainer apDevices;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevices = wifi.Install(phy, mac, apNode);

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
    mobility.Install(sensorNodes);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNode);

    // wireless module ultility
    WirelessModuleUtilityHelper utilityHelper;

    std::vector<std::string> AllInclusionList;
    std::vector<std::string> AllExclusionList;

    AllInclusionList.push_back("ns3::UdpHeader");
    AllExclusionList.push_back("ns3::olsr::PacketHeader");

    utilityHelper.SetInclusionList(AllInclusionList);
    utilityHelper.SetExclusionList(AllExclusionList);

    WirelessModuleUtilityContainer utilities = utilityHelper.InstallAll();

    Ptr<WirelessModuleUtility> utilityPtr = utilities.Get(0);
    utilityPtr->TraceConnectWithoutContext("Rss", MakeCallback(&NodeRss));
    utilityPtr->TraceConnectWithoutContext("Pdr", MakeCallback(&NodePdr));
    utilityPtr->TraceConnectWithoutContext("ThroughputRx", MakeCallback(&NodeThroughputRx));

    // energy model
    BasicEnergySourceHelper basicSourceHelper;
    basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(0.1));
    EnergySourceContainer energySources = basicSourceHelper.Install(sensorNodes);
    WifiRadioEnergyModelHelper radioEnergyHelper;
    radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));
    DeviceEnergyModelContainer deviceModels =
        radioEnergyHelper.Install(staDevices, energySources);
    DeviceEnergyModelContainer jammerDeviceModels =
        radioEnergyHelper.Install(jammerNetdevice.Get(0), energySources.Get(jammerIndex));

    // jammer
    JammerHelper jammerHelper;
    JammerContainer jammers = jammerHelper.Install(jammerNodes);

    // pcap
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcap("output/iotnet", apDevices.Get(0));

    // return
    NetDeviceContainer deviceContainer;
    deviceContainer.Add(apDevices);
    deviceContainer.Add(staDevices);
    return deviceContainer;
}

int main(int argc, char *argv[])
{
    NS_LOG_UNCOND("IoT Network Simulator");

    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // realtime
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));

    // create p2p nodes (n0, n1)
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    // create wifi sensor nodes (n2)
    NodeContainer wifiStaNodes = createSensorNodes(2);

    // get AP node (n0)
    NodeContainer wifiApNode = p2pNodes.Get(0);

    // get server node (n1)
    NodeContainer serverNode;
    serverNode.Add(p2pNodes.Get(1));

    // p2p devices
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    // wifi
    NetDeviceContainer wifiDevices = setupWifi(wifiStaNodes, wifiApNode);

    // internet ipv4 stack
    InternetStackHelper stack;
    stack.Install(p2pNodes);
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);

    // assign ip addresses
    Ipv4AddressHelper address;

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(wifiDevices);

    // server app
    uint16_t sinkPort = 8080;
    Address sinkAddress(InetSocketAddress(p2pInterfaces.GetAddress(1), sinkPort));

    IoTNetServerHelper iotNetServerHelper(sinkAddress);
    ApplicationContainer sinkApps = iotNetServerHelper.Install("c1", p2pDevices.Get(1));
    sinkApps.Start(Seconds(0.));
    sinkApps.Stop(Seconds(10.));

    // client
    IoTNetSensorHelper iotNetSensorHelper(sinkAddress);
    ApplicationContainer sensorApp = iotNetSensorHelper.Install("a1", wifiDevices.Get(1));
    sensorApp.Start(Seconds(1.0));
    sensorApp.Stop(Seconds(9.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // net animation
    AnimationInterface anim("output/wireless-animation.xml");
    anim.EnablePacketMetadata();

    // pcap tracing
    pointToPoint.EnablePcapAll("output/iotnet");

    // run simulation
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}