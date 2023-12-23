/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/node-container.h"
#include "ns3/iotnet.h"

#include "iotnet-server.h"
#include "cpr/cpr.h"

namespace ns3
{
  IoTNetServer::IoTNetServer(const std::string id, const Ipv4Address network, const Ipv4Mask mask, Vector position)
  {
    // create node
    m_node.Create(1);

    // csma
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // ipv4
    m_ipv4.SetBase(network, mask);

    // internet
    IoTNet::world->Add(id, m_node, position);
  }

  void
  IoTNetServer::DataReceivedCallback(Ptr<Socket> socket)
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

      NS_LOG_UNCOND("Received " << packet->GetSize() << " bytes. Payload: ");
      std::cout << payload << std::endl;

      // forward to external server
      cpr::Response r = cpr::Post(
          cpr::Url{"server-node:3000/api/reading"},
          cpr::Header{{"accept", "application/json"}},
          cpr::Header{{"content-type", "application/json"}},
          cpr::Body{{payload}});
    }
  }

  void IoTNetServer::ConnectionAcceptedCallback(Ptr<Socket> socket, const Address &address)
  {
    NS_LOG_UNCOND("Received connection from " << address << std::endl);
    socket->SetRecvCallback(MakeCallback(&IoTNetServer::DataReceivedCallback, this));
  }

  void IoTNetServer::Add(NodeContainer node)
  {
    NetDeviceContainer devices = p2p.Install(NodeContainer(m_node, node));
    Ipv4InterfaceContainer interfaces = m_ipv4.Assign(devices);

    NS_LOG_UNCOND("Ap ip " << interfaces.GetAddress(0));
    NS_LOG_UNCOND("Next hop ip " << interfaces.GetAddress(1));

    uint16_t sinkPort = 8080;
    m_sinkAddress = Address(InetSocketAddress(interfaces.GetAddress(0), sinkPort));

    // application
    Ptr<Socket> socket = Socket::CreateSocket(m_node.Get(0), TcpSocketFactory::GetTypeId());
    socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address &>(), MakeCallback(&IoTNetServer::ConnectionAcceptedCallback, this));
    socket->Bind(m_sinkAddress);
    socket->Listen();

    NS_LOG_UNCOND("Server address " << m_sinkAddress);
  }

  Address IoTNetServer::GetAddress()
  {
    return m_sinkAddress;
  }
}
