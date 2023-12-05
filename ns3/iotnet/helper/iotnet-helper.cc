#include "iotnet-helper.h"

#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-helper.h"

namespace ns3
{

    // local
    static void
    CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
    {
        std::cout << "Cwnd change at " << Simulator::Now().GetSeconds() << " with value " << newCwnd << std::endl;
    }

    static void RssiCallback(Ptr<const WifiPsdu> psdu,
                             RxSignalInfo rxSignalInfo,
                             WifiTxVector txVector,
                             std::vector<bool> statusPerMpdu)
    {
        std::cout << rxSignalInfo << std::endl;
        // psdu->Print(std::cout);
        // std::cout << std::endl;

        // std::cout << psdu->GetPacket() << std::endl;
        // OnDataReceived();

        // Ptr<Packet> packet = psdu->GetPacket();
        // uint8_t buffer[packet->GetSize()];
        // packet->CopyData(buffer, packet->GetSize());

        // // Assuming the payload is a string, you can convert it to a C++ string
        // std::string payload(reinterpret_cast<char *>(buffer), packet->GetSize());

        // std::cout << "Received " << packet->GetSize() << " bytes. Payload: " << payload << std::endl;
    }

    static void
    OnDataReceived(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        while ((packet = socket->Recv()))
        {
            packet->PrintPacketTags(std::cout);

            // Process the received data
            uint8_t buffer[packet->GetSize()];
            packet->CopyData(buffer, packet->GetSize());

            // Assuming the payload is a string, you can convert it to a C++ string
            std::string payload(reinterpret_cast<char *>(buffer), packet->GetSize());

            std::cout << "Received " << packet->GetSize() << " bytes. Payload: " << payload << std::endl;
        }
    }

    static void
    OnConnectionAccepted(Ptr<Socket> socket, const Address &address)
    {
        std::cout << "Received connection from " << address << std::endl;

        socket->SetRecvCallback(MakeCallback(&OnDataReceived));
    }

    /*** IOTNET_SENSOR_HELPER ***/

    // public
    IoTNetSensorHelper::IoTNetSensorHelper(Address address)
    {
        m_factory.SetTypeId(IoTNetSensor::GetTypeId());
        SetAttribute("RemoteAddress", AddressValue(address));
    }

    void
    IoTNetSensorHelper::SetAttribute(std::string name, const AttributeValue &value)
    {
        m_factory.Set(name, value);
    }

    ApplicationContainer
    IoTNetSensorHelper::Install(Ptr<NetDevice> device) const
    {
        return ApplicationContainer(InstallPriv(device));
    }

    // private
    Ptr<Application>
    IoTNetSensorHelper::InstallPriv(Ptr<NetDevice> device) const
    {
        Ptr<Node> node = device->GetNode();
        Ptr<Socket> socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
        socket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

        Ptr<IoTNetSensor> app = m_factory.Create<IoTNetSensor>();
        app->SetSocket(socket);
        node->AddApplication(app);

        Ptr<WifiNetDevice> wifiNetDevice = device->GetObject<WifiNetDevice>();
        Ptr<WifiPhy> wifiPhy = wifiNetDevice->GetPhy();
        Ptr<WifiPhyStateHelper> wifiState = wifiPhy->GetState();

        wifiState->SetTraceReceiveOkCallback(MakeCallback(&RssiCallback));

        return app;
    }

    /*** IOTNET_SERVER_HELPER ***/

    // public
    IoTNetServerHelper::IoTNetServerHelper(Address address)
    {
        m_factory.SetTypeId(IoTNetServer::GetTypeId());
        SetAttribute("Address", AddressValue(address));
    }

    void
    IoTNetServerHelper::SetAttribute(std::string name, const AttributeValue &value)
    {
        m_factory.Set(name, value);
    }

    ApplicationContainer
    IoTNetServerHelper::Install(Ptr<Node> node) const
    {
        return ApplicationContainer(InstallPriv(node));
    }

    // private
    Ptr<Application>
    IoTNetServerHelper::InstallPriv(Ptr<Node> node) const
    {
        Ptr<Socket> socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
        socket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

        Ptr<IoTNetServer> app = m_factory.Create<IoTNetServer>();
        app->SetSocket(socket);
        node->AddApplication(app);

        socket->SetAcceptCallback(
            MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback(&OnConnectionAccepted));

        return app;
    }

}
