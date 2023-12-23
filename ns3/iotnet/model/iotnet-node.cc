#include "ns3/iotnet.h"

#include "iotnet-node.h"

namespace ns3
{
  std::string IoTNetNode::LogPrefix()
  {
    std::stringstream ss;
    ss << "+ [" << id << "](" << Simulator::Now().GetSeconds() << "s): ";
    return ss.str();
  }

  void IoTNetNode::RemainingEnergy(double oldValue, double remainingEnergy)
  {
    NS_LOG_UNCOND(LogPrefix() << "Current remaining energy = " << remainingEnergy << "J");
  }

  void IoTNetNode::TotalEnergy(double oldValue, double totalEnergy)
  {
    NS_LOG_UNCOND(LogPrefix() << "Total energy consumed by radio = " << totalEnergy << "J");
  }

  void IoTNetNode::ScheduleSendData()
  {
    NS_LOG_UNCOND(LogPrefix() << "Schedule send data every 10 seconds to " << IoTNet::world->address);

    SendPacket("hello world");

    Simulator::Schedule(Seconds(1), &IoTNetNode::ScheduleSendData, this);
  }

  void IoTNetNode::SendPacket(std::string message)
  {
    Ptr<Socket> socket = Socket::CreateSocket(node.Get(0), TcpSocketFactory::GetTypeId());
    socket->Bind();
    socket->Connect(IoTNet::world->address);

    Ptr<Packet> packet = Create<Packet>((uint8_t *)message.c_str(), message.length());
    socket->Send(packet);
  }
}