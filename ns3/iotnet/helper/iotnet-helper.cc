#include "iotnet-helper.h"

#include "ns3/names.h"
#include "ns3/uinteger.h"

namespace ns3
{
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
        return ApplicationContainer(InstallPriv("unknown sensor", device));
    }

    ApplicationContainer
    IoTNetSensorHelper::Install(std::string name, Ptr<NetDevice> device) const
    {
        return ApplicationContainer(InstallPriv(name, device));
    }

    // private
    Ptr<Application>
    IoTNetSensorHelper::InstallPriv(std::string name, Ptr<NetDevice> device) const
    {
        Ptr<Node> node = device->GetNode();
        Ptr<IoTNetSensor> app = m_factory.Create<IoTNetSensor>();
        Ptr<Socket> socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
        Ptr<WifiNetDevice> wifiNetDevice = device->GetObject<WifiNetDevice>();
        Ptr<WifiPhy> wifiPhy = wifiNetDevice->GetPhy();
        Ptr<WifiPhyStateHelper> wifiState = wifiPhy->GetState();

        // connect to app
        app->SetName(name);
        app->SetSocket(socket);
        node->AddApplication(app);

        // tracing
        socket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&IoTNetApp::CwndChangeCallback, app));
        wifiState->SetTraceReceiveOkCallback(MakeCallback(&IoTNetSensor::ReceiveOkCallback, app));

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
    IoTNetServerHelper::Install(Ptr<NetDevice> device) const
    {
        return ApplicationContainer(InstallPriv("unknown server", device));
    }

    ApplicationContainer
    IoTNetServerHelper::Install(std::string name, Ptr<NetDevice> device) const
    {
        return ApplicationContainer(InstallPriv(name, device));
    }

    // private
    Ptr<Application>
    IoTNetServerHelper::InstallPriv(std::string name, Ptr<NetDevice> device) const
    {
        Ptr<Node> node = device->GetNode();
        Ptr<IoTNetServer> app = m_factory.Create<IoTNetServer>();
        Ptr<Socket> socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());

        // connect to app
        app->SetName(name);
        app->SetSocket(socket);
        node->AddApplication(app);

        // tracing
        socket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&IoTNetApp::CwndChangeCallback, app));
        socket->SetAcceptCallback(
            MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback(&IoTNetServer::ConnectionAcceptedCallback, app));

        return app;
    }

}
