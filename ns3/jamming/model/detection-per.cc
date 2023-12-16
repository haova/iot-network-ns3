/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

#include "detection-per.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/rng-seed-manager.h"
#include <math.h>



namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("DetectionPer");
NS_OBJECT_ENSURE_REGISTERED (DetectionPer);

TypeId
DetectionPer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DetectionPer")
      .SetParent<Detection> ()
      .AddConstructor<DetectionPer> ()
      .AddAttribute ("DetectionMethod",
                     "Jamming detection method to use.",
                     UintegerValue (0), // default to PDR only
                     MakeUintegerAccessor (&DetectionPer::SetJammingDetectionMethod,
                                           &DetectionPer::GetJammingDetectionMethod),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("DetectionPerDetectionThreshold",
                     "Jamming detection threshold.",
                     DoubleValue (0.9), // default to 0.5
                     MakeDoubleAccessor (&DetectionPer::SetJammingDetectionThreshold,
                                         &DetectionPer::GetJammingDetectionThreshold),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("DetectionPerTxPower",
                     "TX power for channel hop message.",
                     DoubleValue (0.001), // 0.001 W = 0 dBm
                     MakeDoubleAccessor (&DetectionPer::SetTxPower,
                                         &DetectionPer::GetTxPower),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("DetectionPerMessage",
                     "Content of channel hop message.",
                     StringValue ("Channel Hop!"),
                     MakeStringAccessor (&DetectionPer::SetChannelHopMessage,
                                         &DetectionPer::GetChannelHopMessage),
                     MakeStringChecker ())
      .AddAttribute ("DetectionPerDelay",
                     "Channel hop delay.",
                     TimeValue (Seconds (0.0)),
                     MakeTimeAccessor (&DetectionPer::SetChannelHopDelay,
                                       &DetectionPer::GetChannelHopDelay),
                     MakeTimeChecker ())
      .AddAttribute ("DetectionPerSeed",
                     "Seed used in internal RNG.",
                     UintegerValue (12345), // same default defined in rng-stream.h
                     MakeUintegerAccessor (&DetectionPer::SetRngSeed,
                                           &DetectionPer::GetRngSeed),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("DetectionPerStart",
                     "Starting channel number.",
                     UintegerValue (1),   // first available wifi channel number
                     MakeUintegerAccessor (&DetectionPer::SetStartChannelNumber,
                                           &DetectionPer::GetStartChannelNumber),
                     MakeUintegerChecker<uint16_t> ())
      .AddAttribute ("DetectionPerEnd",
                     "Ending channel number.",
                     UintegerValue (11),  // last available wifi channel number
                     MakeUintegerAccessor (&DetectionPer::SetEndChannelNumber,
                                           &DetectionPer::GetEndChannelNumber),
                     MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

DetectionPer::DetectionPer ()
  :  m_rngInitialized (false),
     m_waitingToHop (false)
{
   uint64_t nextStream = RngSeedManager::GetNextStreamIndex ();
   NS_ASSERT(nextStream <= ((1ULL)<<63));
   m_stream = new RngStream (RngSeedManager::GetSeed (),
                                nextStream,
                                RngSeedManager::GetRun ());
}

DetectionPer::~DetectionPer ()
{
}

void
DetectionPer::SetUtility (Ptr<WirelessModuleUtility> utility)
{
  NS_LOG_FUNCTION (this << utility);
  NS_ASSERT (utility != NULL);
  m_utility = utility;
}

void
DetectionPer::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}

void
DetectionPer::SetJammingDetectionMethod (JammingDetectionMethod method)
{
  NS_LOG_FUNCTION (this << method);
  m_jammingDetectionMethod = method;
}

uint32_t
DetectionPer::GetJammingDetectionMethod (void) const
{
  NS_LOG_FUNCTION (this);
  return m_jammingDetectionMethod;
}

void
DetectionPer::SetJammingDetectionThreshold (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_jammingDetectionThreshold = threshold;
}

double
DetectionPer::GetJammingDetectionThreshold (void) const
{
  NS_LOG_FUNCTION (this);
  return m_jammingDetectionThreshold;
}

void
DetectionPer::SetTxPower (double txPower)
{
  NS_LOG_FUNCTION (this << txPower);
  m_txPower = txPower;
}

double
DetectionPer::GetTxPower (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txPower;
}

void
DetectionPer::SetChannelHopMessage (std::string message)
{
  NS_LOG_FUNCTION (this);
  m_channelHopMessage = message;
}

std::string
DetectionPer::GetChannelHopMessage (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channelHopMessage;
}

void
DetectionPer::SetChannelHopDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_channelHopDelay = delay;
}

Time
DetectionPer::GetChannelHopDelay (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channelHopDelay;
}

double
DetectionPer::DegreeOfJamming (int method)
{
  NS_LOG_FUNCTION (this << method);
  NS_LOG_DEBUG ("Detection:At Node #" << GetId () <<
                ", Calculating degree of jamming!");

  /*
   * Here we use RSS value at the start of packet as reference for calculating
   * RSS ratio. This is effective for reactive jammers where the RSS is low at
   * the beginning of packet, but the average RSS is higher. However this may
   * not be effective for constant jammers, which results a constantly high RSS
   * value and a low RSS ratio.
   */
  double rssRatio = fabs (m_averageRss - m_startRss) / m_startRss;
  switch (method)
    {
    case PDR_ONLY:
      NS_LOG_DEBUG ("DetectionPer:At Node #" << GetId () <<
                    ", PDR only!" << m_utility->GetPdr () );

      return  m_utility->GetPdr ();
    case RSS_ONLY:
      NS_LOG_DEBUG ("DetectionPer:At Node #" << GetId () <<
                    ", RSS only!");
      return (rssRatio > 1 ? 1 : rssRatio);  // max = 1
    case PDR_AND_RSS:
      NS_LOG_DEBUG ("DetectionPer:At Node #" << GetId () <<
                    ", PDR & RSS!");
      // return the average of RSS & PDR
      return ((rssRatio + (1 - m_utility->GetPdr ())) / 2);
    default:
      NS_FATAL_ERROR ("DetectionPer:At Node #" << GetId () <<
                      ", Unknown jamming detection method!");
      break;
    }
  return -1.0;  // error
}

bool
DetectionPer::IsJammingDetected (int method)
{

  NS_LOG_FUNCTION (this << method);
  NS_LOG_DEBUG ("DetectionPer:At Node #" << GetId () <<
                ", Deciding if jamming is detected!" << m_jammingDetectionThreshold);
    

  double degreeOfJamming = DegreeOfJamming (method);
  if (degreeOfJamming < m_jammingDetectionThreshold)
    {
      NS_LOG_DEBUG ("DetectionPer:At Node #" << GetId () <<
                    ", Jamming is detected!");
      return true;
    }
  else
    {
      NS_LOG_DEBUG ("DetectionPer:At Node #" << GetId () <<
                    ", Jamming is NOT detected!");
      return false;
    }
}

void
DetectionPer::SendChannelHopMessage(void)
{
 NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                ", Sending channel hop message >> \n--\n" << m_channelHopMessage <<
                "\n--");

  // build mitigation packet
  Ptr<Packet> packet = Create<Packet> ((uint8_t *)m_channelHopMessage.c_str (),
                                       m_channelHopMessage.size ());

  NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                ", Sending channel hop packet with power = " << m_txPower << " W");

  // send mitigation signal
  double actualPower = m_utility->SendMitigationMessage (packet, m_txPower);
  if (actualPower != 0.0)
    {
      NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                    ", Channel hop packet sent with power = " << actualPower << " W");
    }
  else
    {
      NS_LOG_ERROR ("MitigateByChannelHop:At Node #" << GetId () <<
                    ", Failed to send channel hop packet!");
    }

  m_waitingToHop = true;  // set waiting for channel hop flag*/

  NS_LOG_FUNCTION (this);

  NS_LOG_FUNCTION ("jamming detecter at" << Simulator::Now ().GetSeconds() << "s");
}

void
DetectionPer::HopChannel (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_waitingToHop); // make sure we are waiting to hop channel

  // calculate channel number to hop to
  uint16_t channelNumber = RandomSequenceGenerator ();

  // schedule hop channel after sending is complete
  Simulator::Schedule (m_channelHopDelay,
      &ns3::WirelessModuleUtility::SwitchChannel, m_utility, channelNumber);

  NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () << ", Hopping from " <<
                m_utility->GetPhyLayerInfo().currentChannel << " >-> " <<
                channelNumber << ", At " << Simulator::Now ().GetSeconds () << "s");

  m_waitingToHop = false; // reset flag after channel hop
}

void
DetectionPer::SetRngSeed (uint32_t seed)
{
  NS_LOG_FUNCTION (this << seed);
  m_seed = seed;
  // m_stream.SetPackageSeed (seed);
  // m_stream.InitializeStream ();
}

uint32_t
DetectionPer::GetRngSeed (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seed;
}

void
DetectionPer::SetStartChannelNumber (uint16_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  m_channelStart = channelNumber;
}

uint16_t
DetectionPer::GetStartChannelNumber (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channelStart;
}

void
DetectionPer::SetEndChannelNumber (uint16_t channelNumber)
{
  NS_LOG_FUNCTION (this);
  m_channelEnd = channelNumber;
}

uint16_t
DetectionPer::GetEndChannelNumber (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channelEnd;
}

/*
 * Private functions start here.
 */

void
DetectionPer::DoStart (void)
{
  NS_LOG_FUNCTION (this);
  StartDetection (); // start mitigation at beginning of simulation
}

void
DetectionPer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Detection: At node #" << GetId () <<
                " Current channel number = " <<
                m_utility->GetPhyLayerInfo ().currentChannel);
  StopDetection ();
}

void
DetectionPer::DoDetection (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Detection:At Node #" << GetId () <<
                ", Mitigation started!");
}

void
DetectionPer::DoStopDetection (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                ", Mitigation stopped!");
}

void
DetectionPer::DoStartRxHandler (Ptr<Packet> packet, double startRss)
{
  NS_LOG_FUNCTION (this << packet << startRss);
  m_startRss = startRss;
}

void
DetectionPer::DoEndRxHandler (Ptr<Packet> packet, double averageRss)
{
  NS_LOG_FUNCTION (this << packet << averageRss);
  NS_LOG_DEBUG ("DetectionPer:At Node #" << GetId () <<
                ", Handling incoming packet!");

  if (!IsDetectionOn ())
    {
      NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                    ", Mitigation is OFF!");
      return;
    }

  m_averageRss = averageRss;

  // detect jamming
  if (IsJammingDetected (m_jammingDetectionMethod))
    {
      NS_LOG_DEBUG ("DetectionPer:At Node #" << GetId () <<
                    ", Sending channel hop message at " <<
                    Simulator::Now ().GetSeconds () << "s");
      SendChannelHopMessage ();
    }

  if (packet == NULL)
    {
      NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                    ", Failed to receive current packet!");
      return;
    }

  // check if incoming packet is channel hop message
   if (FindMessage (packet, m_channelHopMessage))
    {
      if (!m_waitingToHop)  // hop only if current node is not waiting to hop
        {
          NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                        ", Received channel hop message at " <<
                        Simulator::Now ().GetSeconds ());
         
          NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                        ", Passing channel hop message!");
          SendChannelHopMessage ();
        }
      else
        {
          NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                        ", Received channel hop message!" <<
                        " But not hopping because current node is waiting to hop!");
        }
    }
}

void
DetectionPer::DoEndTxHandler (Ptr<Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);

  if (m_waitingToHop) // check waiting for channel hop flag
    {
      NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                    ", schedule hop at end of TX!");
      // schedule channel hop after some time
      Simulator::Schedule (m_channelHopDelay, &DetectionPer::HopChannel,
                           this);
    }
  else
    {
      NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                    ", Not doing anything at end of TX!");
    }
}

bool
DetectionPer::FindMessage (Ptr<const Packet> packet, std::string target) const
{
  NS_LOG_FUNCTION (this << packet);
  NS_ASSERT (packet != NULL);

  if (packet->GetSize () != target.size ()) // check size of packet first
    {
      NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                    ", Size incorrect, not checking data!");
      return false;
    }

  // copy data from packet
  uint8_t data[packet->GetSize()];
   packet->CopyData (data, packet->GetSize ());

   //convert data to string
   std::string dataString;
   dataString.assign ((char*)data, packet->GetSize ());

  // show packet content
  NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () <<
                ", Packet size = " << packet->GetSize () << " content is:\n---\n" <<
                dataString << "\n---");

  return (dataString.compare (target) == 0);  // check if is channel hop message

  return true;
}

uint16_t
DetectionPer::RandomSequenceGenerator (void)
{
  NS_LOG_FUNCTION (this);
  double nb_aletoire = m_stream->RandU01();
  uint32_t nb_channel = (m_channelStart - m_channelEnd) + 1 ;
  double nb_divisible = 1/nb_channel;
  double res = nb_aletoire / nb_divisible;
  uint32_t arrondi = floor(res);
  uint16_t channelNumber = arrondi + m_channelStart;

  NS_LOG_DEBUG ("MitigateByChannelHop:At Node #" << GetId () << ", RNG returned " <<
                channelNumber);
  return channelNumber;
}

} // namespace ns3
