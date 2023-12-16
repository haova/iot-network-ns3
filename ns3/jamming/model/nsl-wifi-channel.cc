#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "nsl-wifi-channel.h"
#include "nsl-wifi-phy.h"
#include "ns3/mobility-model.h"
#include "ns3/wifi-utils.h"



namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NslWifiChannel");
NS_OBJECT_ENSURE_REGISTERED (NslWifiChannel);

TypeId
NslWifiChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NslWifiChannel")
    .SetParent<Channel> ()
    .SetGroupName ("Wifi")
    .AddConstructor<NslWifiChannel> ()
    .AddAttribute ("PropagationLossModel", "A pointer to the propagation loss model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&NslWifiChannel::m_loss),
                   MakePointerChecker<PropagationLossModel> ())
    .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&NslWifiChannel::m_delay),
                   MakePointerChecker<PropagationDelayModel> ())
  ;
  return tid;
}

NslWifiChannel::NslWifiChannel ()
{
  NS_LOG_FUNCTION (this);
}

NslWifiChannel::~NslWifiChannel ()
{
  NS_LOG_FUNCTION (this);
  m_phyList.clear ();
}

void
NslWifiChannel::SetPropagationLossModel (const Ptr<PropagationLossModel> loss)
{
  NS_LOG_FUNCTION (this << loss);
  m_loss = loss;
}

void
NslWifiChannel::SetPropagationDelayModel (const Ptr<PropagationDelayModel> delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_delay = delay;
}

void
NslWifiChannel::Send (Ptr<NslWifiPhy> sender, Ptr<const Packet> packet, double txPowerDbm, Time duration,WifiPreamble preamble,WifiTxVector txVector) const
{
  NS_LOG_FUNCTION (this << sender << packet << txPowerDbm << duration.GetSeconds ());
  Ptr<MobilityModel> senderMobility = sender->GetMobility ();
  NS_ASSERT (senderMobility != 0);
  NS_LOG_FUNCTION (sender->GetChannelNumber());
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++)
    {
      if (sender != (*i))
        {
          //For now don't account for inter channel interference nor channel bonding
          if ((*i)->GetChannelNumber () != sender->GetChannelNumber ())
            {
              continue;
            }

          Ptr<MobilityModel> receiverMobility = (*i)->GetMobility ()->GetObject<MobilityModel> ();
          Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
          double rxPowerDbm = m_loss->CalcRxPower (txPowerDbm, senderMobility, receiverMobility);
          NS_LOG_DEBUG ("propagation: txPower=" << txPowerDbm << "dbm, rxPower=" << rxPowerDbm << "dbm, " <<
                        "distance=" << senderMobility->GetDistanceFrom (receiverMobility) << "m, delay=" << delay);
          Ptr<Packet> copy = packet->Copy ();
          Ptr<NetDevice> dstNetDevice = (*i)->GetDevice ();
          uint32_t dstNode;
          if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
          else
            {
              dstNode = dstNetDevice->GetNode ()->GetId ();
            }

          Simulator::ScheduleWithContext (dstNode,
                                          delay, &NslWifiChannel::Receive,
                                          (*i), copy, rxPowerDbm, duration,preamble,txVector);
        }
    }
}

void
NslWifiChannel::Receive (Ptr<NslWifiPhy> phy, Ptr<Packet> packet, double rxPowerDbm, Time duration, WifiPreamble preamble,WifiTxVector txVector)
{
  NS_LOG_FUNCTION (phy << packet << rxPowerDbm << duration.GetSeconds ());
  // Do no further processing if signal is too weak
  // Current implementation assumes constant rx power over the packet duration
  if ((rxPowerDbm + phy->GetRxGain ()) < phy->GetRxSensitivity ())
    {
      NS_LOG_INFO ("Received signal too weak to process: " << rxPowerDbm << " dBm");
      return;
    }
  //phy->StartReceivePreamble (packet, DbmToW (rxPowerDbm + phy->GetRxGain ()), duration);
  phy->StartReceivePacket (packet, rxPowerDbm, txVector,preamble);
}

std::size_t
NslWifiChannel::GetNDevices (void) const
{
  return m_phyList.size ();
}

Ptr<NetDevice>
NslWifiChannel::GetDevice (std::size_t i) const
{
  return m_phyList[i]->GetDevice ()->GetObject<NetDevice> ();
}

void
NslWifiChannel::Add (Ptr<NslWifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  m_phyList.push_back (phy);
}

int64_t
NslWifiChannel::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  int64_t currentStream = stream;
  currentStream += m_loss->AssignStreams (stream);
  return (currentStream - stream);
}



} //namespace ns3
