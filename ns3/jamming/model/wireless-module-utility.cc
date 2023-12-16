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

#include "wireless-module-utility.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/pointer.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/trace-source-accessor.h"
#include <math.h>

NS_LOG_COMPONENT_DEFINE ("WirelessModuleUtility");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WirelessModuleUtility);

TypeId
WirelessModuleUtility::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WirelessModuleUtility")
    .SetParent<Object> ()
    .AddConstructor<WirelessModuleUtility> ()
    .AddAttribute ("ThroughputUpdateInterval",
                   "Time window for calculation of derivative of total bytes (throughput).",
                   TimeValue (Seconds (1.0)),    // default to 1.0 second
                   MakeTimeAccessor (&WirelessModuleUtility::SetThroughputUpdateInterval,
                                     &WirelessModuleUtility::GetThroughputUpdateInterval),
                   MakeTimeChecker ())
    .AddAttribute ("PdrWindowSize",
                   "Number of packets to be considered as a moving window for PDR measurements.",
                   UintegerValue (1600),          // default to 40 packets
                   MakeUintegerAccessor (&WirelessModuleUtility::SetPdrWindowSize,
                                         &WirelessModuleUtility::GetPdrWindowSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RssUpdateInterval",
                   "Update interval of RSS values.",
                   TimeValue (Seconds (0.5)),   // default to 0.5 second
                   MakeTimeAccessor (&WirelessModuleUtility::SetRssUpdateInterval,
                                     &WirelessModuleUtility::GetRssUpdateInterval),
                   MakeTimeChecker ())
    .AddTraceSource ("TotalBytesRx",
                     "Total bytes received at current node.",
                     MakeTraceSourceAccessor (&WirelessModuleUtility::m_totalBytesRx),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("TotalBytesTx",
                     "Total bytes sent at current node.",
                     MakeTraceSourceAccessor (&WirelessModuleUtility::m_totalBytesTx),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("ThroughputRx",
                     "RX throughput at current node.",
                     MakeTraceSourceAccessor (&WirelessModuleUtility::m_throughputRx),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("ThroughputTx",
                     "TX throughput at current node.",
                     MakeTraceSourceAccessor (&WirelessModuleUtility::m_throughputTx),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Pdr",
                     "Packet Delivery Rate at current node.",
                     MakeTraceSourceAccessor (&WirelessModuleUtility::m_Pdr),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Rss",
                     "Received Signal Strength at current node.",
                     MakeTraceSourceAccessor (&WirelessModuleUtility::m_nodeRssW),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PacketRss",
                     "Received Signal Strength per packet at current node.",
                     MakeTraceSourceAccessor (&WirelessModuleUtility::m_avgPktRssW),
                     "ns3::Packet::TracedCallback")
    ;
  return tid;
}

WirelessModuleUtility::WirelessModuleUtility (void)
  :  m_totalBytesRx (0),
     m_totalBytesTx (0),
     m_previousTotalBytesRx (0),
     m_previousTotalBytesTx (0),
     m_throughputRx (0),
     m_throughputTx (0),
     m_Pdr (0),
     m_numOfPktsRecvd (0),
     m_pdrArrayCurIndex (0),
     m_nodeRssW (0),
     m_totalPkts (0),
     m_validPkts (0)
{
  m_pktStatusRecord.clear ();
  m_rssMeasurementCallback.Nullify ();
  m_sendPacketCallback.Nullify ();
  m_startRxCallback.Nullify ();
  m_startTxCallback.Nullify ();
  m_endRxCallback.Nullify ();
  m_channelSwitchCallback.Nullify ();
}

WirelessModuleUtility::~WirelessModuleUtility (void)
{
}

void
WirelessModuleUtility::SetThroughputUpdateInterval (Time updateInterval)
{
  NS_LOG_FUNCTION (this << updateInterval);
  NS_ASSERT (updateInterval.GetSeconds () >= 0);
  m_throughputUpdateInterval = updateInterval;
  // cancel previously scheduled throughput update event
  m_throughputUpdateEvent.Cancel ();
  // update previous total bytes
  m_previousTotalBytesTx = m_totalBytesTx;
  m_previousTotalBytesRx = m_totalBytesRx;
  // schedule the next recording of throughput
  if (!Simulator::IsFinished ())
    {
      m_throughputUpdateEvent = Simulator::Schedule (m_throughputUpdateInterval,
                                                     &WirelessModuleUtility::UpdateThroughput,
                                                     this);
    }
}

Time
WirelessModuleUtility::GetThroughputUpdateInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_throughputUpdateInterval;
}

void
WirelessModuleUtility::SetPdrWindowSize (uint32_t pdrWindowSize)
{
  NS_LOG_FUNCTION (this << pdrWindowSize);
  NS_ASSERT (pdrWindowSize != 0);
  m_pdrWindowSize = pdrWindowSize;
  // allocate packet status record list upon changing PDR window size
  m_pktStatusRecord.clear ();
  m_pktStatusRecord.assign (m_pdrWindowSize, false);
}

uint32_t
WirelessModuleUtility::GetPdrWindowSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_pdrWindowSize;
}

void
WirelessModuleUtility::SetRssUpdateInterval (Time updateInterval)
{
  NS_LOG_FUNCTION (this << updateInterval);
  m_rssUpdateInterval = updateInterval;
}

Time
WirelessModuleUtility::GetRssUpdateInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rssUpdateInterval;
}

void
WirelessModuleUtility::SetRssMeasurementCallback (UtilityRssCallback RssCallback)
{
  NS_LOG_FUNCTION (this);
  m_rssMeasurementCallback = RssCallback;
}

void
WirelessModuleUtility::SetInclusionList (std::vector<std::string> list)
{
  NS_LOG_FUNCTION (this);
  m_headerInclusionList = list;
}

void
WirelessModuleUtility::SetExclusionList (std::vector<std::string> list)
{
  NS_LOG_FUNCTION (this);
  m_headerExclusionList = list;
}

void
WirelessModuleUtility::SetSendPacketCallback (UtilitySendPacketCallback sendPacketCallback)
{
  NS_LOG_FUNCTION (this);
  m_sendPacketCallback = sendPacketCallback;
}

void
WirelessModuleUtility::SetChannelSwitchCallback (UtilityChannelSwitchCallback channelSwitchCallback)
{
  NS_LOG_FUNCTION (this);
  m_channelSwitchCallback = channelSwitchCallback;
}

void
WirelessModuleUtility::SetStartTxCallback (UtilityTxCallback startTxCallback)
{
  NS_LOG_FUNCTION (this);
  m_startTxCallback = startTxCallback;
}

void
WirelessModuleUtility::SetEndTxCallback (UtilityTxCallback endTxCallback)
{
  NS_LOG_FUNCTION (this);
  m_endTxCallback = endTxCallback;
}

void
WirelessModuleUtility::SetStartRxCallback (UtilityRxCallback startRxCallback)
{
  NS_LOG_FUNCTION (this);
  m_startRxCallback = startRxCallback;
}

void
WirelessModuleUtility::SetEndRxCallback (UtilityRxCallback endRxCallback)
{
  NS_LOG_FUNCTION (this);
  m_endRxCallback = endRxCallback;
}

void
WirelessModuleUtility::SetPhyLayerInfo (PhyLayerInfo info)
{
  NS_LOG_FUNCTION (this);
  m_phyLayerInfo = info;
}

uint32_t
WirelessModuleUtility::GetValidPkts ()
{
  NS_LOG_FUNCTION (this);
  return m_validPkts;
}

uint32_t
WirelessModuleUtility::GetTotalPkts ()
{
  NS_LOG_FUNCTION (this);
  return m_totalPkts;
}


WirelessModuleUtility::PhyLayerInfo
WirelessModuleUtility::GetPhyLayerInfo (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phyLayerInfo;
}

bool
WirelessModuleUtility::StartRxHandler (Ptr<Packet> packet, double startRssW)
{
  NS_LOG_FUNCTION (this << packet << startRssW);

  UpdateRss ();

  // notify jammer or jamming mitigation
  if (!m_startRxCallback.IsNull ())
    {
      return m_startRxCallback (packet, startRssW);
    }
  else
    {
      NS_LOG_DEBUG ("WirelessModuleUtility:StartRxHandler is not installed!");
      return true;
    }
}

void
WirelessModuleUtility::EndRxHandler (Ptr<Packet> packet,
                                     double averageRssW,
                                     const bool isSuccessfullyReceived)
{
  NS_LOG_FUNCTION (this << packet << averageRssW << isSuccessfullyReceived);
  NS_LOG_DEBUG ("WirelessModuleUtility:Handling received packet from PHY!");

  // Update the RSS since the value will change when the packet ends.
  UpdateRss();
  
  AnalyzeAndRecordIncomingPacket (packet, isSuccessfullyReceived);

  m_avgPktRssW = averageRssW;

  // notify jammer or mitigation module
  if (m_endRxCallback.IsNull ())
    {
      NS_LOG_DEBUG ("WirelessModuleUtility:Brain is not installed!");
    }
  else
    {
      if (isSuccessfullyReceived)
        {
          NS_LOG_DEBUG ("WirelessModuleUtility:Sucees");
          m_endRxCallback (packet, m_avgPktRssW);
        }
      else
        {
          NS_LOG_DEBUG ("WirelessModuleUtility:NoSucees");
          m_endRxCallback (NULL, m_avgPktRssW); // pass NULL if packet is corrupted
        }
    }
}

void
WirelessModuleUtility::StartTxHandler (Ptr<const Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);
  AnalyzeAndRecordOutgoingPacket (packet);
if (m_endTxCallback.IsNull ())
    {
      NS_LOG_DEBUG ("WirelessModuleUtility:Start TX callback is not installed!");
    }
  else
   {
      Ptr<Packet> copy = packet->Copy ();
      m_endTxCallback (copy, txPower);
   }
}

void
WirelessModuleUtility::EndTxHandler (Ptr<const Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);

  if (m_endTxCallback.IsNull ())
    {
      NS_LOG_DEBUG ("WirelessModuleUtility:End TX callback is not installed!");
    }
  else
    {
      Ptr<Packet> copy = packet->Copy ();
      m_endTxCallback (copy, txPower);
    }
}

uint64_t
WirelessModuleUtility::GetTotalBytesRx (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalBytesRx;
}

uint64_t
WirelessModuleUtility::GetTotalBytesTx (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalBytesTx;
}

double
WirelessModuleUtility::GetRxThroughput (void) const
{
  NS_LOG_FUNCTION (this);
  return m_throughputRx;
}

double
WirelessModuleUtility::GetTxThroughput (void) const
{
  NS_LOG_FUNCTION (this);
  return m_throughputTx;
}

double
WirelessModuleUtility::GetPdr (void) const
{
  NS_LOG_FUNCTION (this);
  return m_Pdr;
}

double
WirelessModuleUtility::GetRss (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nodeRssW;
}

/*
 * Private functions start here!
 */

void
WirelessModuleUtility::DoStart (void)
{
  NS_LOG_FUNCTION (this);
  UpdateRss ();         // start RSS update at beginning of simulation
  UpdateThroughput ();  // start throughput update at beginning of simulation
}

void
WirelessModuleUtility::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  // show total bytes at end of simulation
  NS_LOG_DEBUG ("WirelessModuleUtility:Total bytes RX = " << m_totalBytesRx);
  NS_LOG_DEBUG ("WirelessModuleUtility:Total bytes TX = " << m_totalBytesTx);
}

void
WirelessModuleUtility::AnalyzeAndRecordIncomingPacket (const Ptr<Packet> packet,
    bool isSuccessfullyReceived)
{
  NS_LOG_FUNCTION (this << packet << isSuccessfullyReceived);
  NS_LOG_FUNCTION (packet->GetSize());
  NS_LOG_FUNCTION(FindHeaderInInclusionList(packet));
  NS_LOG_FUNCTION(FindHeaderInExclusionList(packet));

  if (isSuccessfullyReceived)
    {
      if (FindHeaderInInclusionList (packet) && !FindHeaderInExclusionList (packet))
        {
          m_totalBytesRx += packet->GetSize ();
        }
      UpdatePdr (true);   // update PDR for successful delivery
    }
  else
    {
      UpdatePdr (false);  // update PDR for delivery failure
    }
}

void
WirelessModuleUtility::AnalyzeAndRecordOutgoingPacket (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  NS_LOG_INFO("tes avant");

  /*if (FindHeaderInInclusionList (packet) && !FindHeaderInExclusionList (packet))
    {*/
      NS_LOG_INFO("tes dans fonction");
      m_totalBytesTx += packet->GetSize ();
      NS_LOG_INFO("tes dans fonction" << m_totalBytesTx);
    /*}*/
}

void WirelessModuleUtility::UpdateThroughput (void)
{
  NS_LOG_FUNCTION (this);

  // cancel previously scheduled throughput update event
  m_throughputUpdateEvent.Cancel ();

  double throughputUpdateIntervalS = m_throughputUpdateInterval.GetSeconds ();
  uint64_t txChange = m_totalBytesTx - m_previousTotalBytesTx;
  uint64_t rxChange = m_totalBytesRx - m_previousTotalBytesRx;

  // calculate throughput in bits per seconds
  m_throughputTx = (double)(txChange * 8) / throughputUpdateIntervalS;
  m_throughputRx = (double)(rxChange * 8) / throughputUpdateIntervalS;

  // step to next update window
  m_previousTotalBytesTx = m_totalBytesTx;
  m_previousTotalBytesRx = m_totalBytesRx;

  // schedule next update
  m_throughputUpdateEvent = Simulator::Schedule (m_throughputUpdateInterval,
                                                 &WirelessModuleUtility::UpdateThroughput,
                                                 this);
}

uint32_t
WirelessModuleUtility::FindHeader (Ptr<const Packet> packet,
                                   std::string headerName) const
{
  NS_LOG_FUNCTION (this << packet);
  Packet copy = *packet;  // make a copy of the header
  NS_LOG_DEBUG ("WirelessModuleUtility:Packet has the following headers/trailers:" <<
                "\n----------\n" << copy << "\n----------");

  // iterate through all items in the header
  PacketMetadata::ItemIterator pktItemIterator = copy.BeginItem ();
  while (pktItemIterator.HasNext ())
    {
      PacketMetadata::Item item = pktItemIterator.Next ();
      if (item.isFragment)
        {
          switch (item.type)
            {
            case PacketMetadata::Item::PAYLOAD:
              break;
            case PacketMetadata::Item::HEADER:
            case PacketMetadata::Item::TRAILER:
              std::string itemName = item.tid.GetName ();
              uint32_t size = item.currentSize; // obtain size of header/trailer
              // check if we found header string
              if (headerName.compare (itemName) == 0)
                {
                  NS_LOG_DEBUG ("WirelessModuleUtility:" << itemName <<
                                " Found!" << " Size = " << size << " bytes");
                  return size;
                }
              break;
            }
          NS_LOG_DEBUG ("WirelessModuleUtility:" << " Fragment [" <<
                        item.currentTrimedFromStart <<":" <<
                        (item.currentTrimedFromStart + item.currentSize) << "]");
        } // end if
      else
        {
          switch (item.type)
            {
            case PacketMetadata::Item::PAYLOAD:
              NS_LOG_DEBUG ("WirelessModuleUtility:" << "Payload (size=" <<
                            item.currentSize << ")");
              break;
            case PacketMetadata::Item::HEADER:
            case PacketMetadata::Item::TRAILER:
              std::string itemName = item.tid.GetName ();
              uint32_t size = item.currentSize; // obtain size of header/trailer
              // check if we found header string
              if (headerName.compare (itemName) == 0)
                {
                  NS_LOG_DEBUG ("WirelessModuleUtility:" << itemName <<
                                " Found!" << " Size = " << size << " bytes");
                  return size;  // header/trailer found
                }
              // skip checking of header/trailer contents
              break;
            }
        } // end else
    } // end while
  return 0; // return 0 if not found
}

bool
WirelessModuleUtility::FindHeaderInInclusionList (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  /*
   * Return true if list is empty. The inclusion list is default to be empty,
   * that is, we will include packets with any header/trailer.
   */
  if (m_headerInclusionList.empty ())
    {
      NS_LOG_FUNCTION ("findheader");

      return true;
    }

  // iterate through the header/trailer inclusion list
  std::vector<std::string>::iterator listItr = m_headerInclusionList.begin ();
  while (listItr != m_headerInclusionList.end ())
    {
      if (FindHeader (packet, *listItr) != 0)
        {
          NS_LOG_FUNCTION ("findheader2");
          return true;
        }
      listItr++;
    }
    NS_LOG_FUNCTION ("findheader3");
  return false;
}

bool
WirelessModuleUtility::FindHeaderInExclusionList (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // iterate through the header/trailer exclusion list
  std::vector<std::string>::iterator listItr = m_headerExclusionList.begin ();
  while (listItr != m_headerExclusionList.end ())
    {
      if (FindHeader (packet, *listItr) != 0)
        {
          return true;
        }
      listItr++;
    }
  return false;
}

void
WirelessModuleUtility::UpdatePdr (const bool isPacketValid)
{
  NS_LOG_FUNCTION (this << isPacketValid);
  uint32_t validPkts = 0;
  uint32_t totalPkts = 0;

  // create m_pktStatusRecord list ONLY if empty (initialization)
  if (m_pktStatusRecord.empty ())
    {
      m_pktStatusRecord.assign (m_pdrWindowSize, false);
    }

  // insert current packet status into PDR status list
  InsertIntoPdrArray (isPacketValid);

  // calculate how many packets the PDR is to be calculated with.
  if (m_numOfPktsRecvd > m_pdrWindowSize)
    {
      
      totalPkts = m_pdrWindowSize;
    }
  else
    {
      totalPkts = m_numOfPktsRecvd;
    }

  // Count the number of valid packets in the array
  validPkts = 0;
  for (uint32_t i = 0; i < totalPkts ; i++)
    {
      if (m_pktStatusRecord[i])
        {
          validPkts++;
        }
    }
  // calculate PDR
  m_Pdr = (double) validPkts / (double) totalPkts;

  NS_LOG_DEBUG ("WirelessModuleUtility:Updated PDR = " << m_Pdr <<
                ", Total received packets = " << totalPkts <<
                ", # of valid packets = "<< validPkts);
  
  m_totalPkts = totalPkts;
  m_validPkts = validPkts;
}


void
WirelessModuleUtility::InsertIntoPdrArray (bool newPacketStatus)
{
  NS_LOG_FUNCTION (this << newPacketStatus);
  /*
   * Insert the status of the latest packet at the current index of array and
   * increment the current index to be ready for next insertion.
   */
  m_pktStatusRecord[m_pdrArrayCurIndex++] = newPacketStatus;
  m_numOfPktsRecvd++;

  // wrap around the array
  if (m_pdrArrayCurIndex == m_pdrWindowSize)
    {
      m_pdrArrayCurIndex = 0;
    }
}

void
WirelessModuleUtility::UpdateRss (void)
{
  NS_LOG_FUNCTION (this);

  m_updateRssEvent.Cancel (); // cancel event if exists

  if (m_rssMeasurementCallback.IsNull ())
    {
      NS_LOG_ERROR ("WirelessModuleUtility:RSS measurement callback not set!");
      return;
    }

  m_nodeRssW = m_rssMeasurementCallback (); // calculate & set RSS

  NS_LOG_DEBUG ("WirelessModuleUtility:At time = " << Simulator::Now().GetSeconds () <<
                " s" << ", Current RSS = " << m_nodeRssW << " W" << ", in dBm " <<
                WToDbm (m_nodeRssW));

  // schedule next update
  m_updateRssEvent = Simulator::Schedule (m_rssUpdateInterval,
                                          &WirelessModuleUtility::UpdateRss,
                                          this);
}

double
WirelessModuleUtility::SendJammingSignal (double power, Time duration)
{
  NS_LOG_FUNCTION (this << power << duration);

  // Check that the power is within allowed range of PHY layer
  if (power > m_phyLayerInfo.maxTxPowerW)
    {
      NS_LOG_ERROR ("WirelessModuleUtility:Power of "<< power <<
                    " W is not supported by PHY layer, max power = " <<
                    m_phyLayerInfo.maxTxPowerW << ", scaling to max power!");
      power = m_phyLayerInfo.maxTxPowerW;
    }
  else if (power < m_phyLayerInfo.minTxPowerW)
    {
      NS_LOG_ERROR ("WirelessModuleUtility:Power of "<< power <<
                    " W is not supported by PHY layer, min power = " <<
                    m_phyLayerInfo.minTxPowerW << ", scaling to min power!");
      power = m_phyLayerInfo.minTxPowerW;
    }

  /*
   * Convert the desired signal length into a packet of corresponding size for
   * sending through PHY layer.
   */
  uint32_t numBytes = duration.GetSeconds() * m_phyLayerInfo.phyRate / 8 ;
 
  NS_LOG_INFO(m_phyLayerInfo.phyRate);
   NS_LOG_INFO(duration.GetSeconds());
  NS_LOG_INFO("test" << numBytes);

   NS_LOG_INFO(power);
  
  if (m_sendPacketCallback.IsNull ())
    {
      NS_FATAL_ERROR ("WirelessModuleUtility:Send packet callback is NOT set!");
    }
  // send jamming signal
  m_sendPacketCallback (Create<Packet> (numBytes), power, SEND_AS_JAMMER);

  return power;
}

void
WirelessModuleUtility::SwitchChannel (uint16_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);

  /*
   * Check if callback is setup. If PHY does not support channel switch, this
   * callback will be NULL.
   */
  if (m_channelSwitchCallback.IsNull ())
    {
      NS_FATAL_ERROR ("WirelessModuleUtility:Channel switch callback not set!");
    }
  else
    {
      NS_LOG_FUNCTION ("je passe ici ");
      m_channelSwitchCallback (channelNumber);
    }
}

double
WirelessModuleUtility::SendMitigationMessage (Ptr<Packet> packet, double power)
{
  NS_LOG_FUNCTION (this << packet << power);

  // Check that the power is within allowed range of PHY layer
  if (power > m_phyLayerInfo.maxTxPowerW)
    {
      NS_LOG_ERROR ("WirelessModuleUtility:Power of "<< power <<
                    " W is not supported by PHY layer, max power = " <<
                    m_phyLayerInfo.maxTxPowerW << ", scaling to max power!");
      power = 0.04;
    }
  else if (power < m_phyLayerInfo.minTxPowerW)
    {
      NS_LOG_ERROR ("WirelessModuleUtility:Power of "<< power <<
                    " W is not supported by PHY layer, min power = " <<
                    m_phyLayerInfo.minTxPowerW << ", scaling to min power!");
      power = 0.04;
    }

  NS_LOG_INFO(m_sendPacketCallback.IsNull());
  // check if callback is installed
  if (m_sendPacketCallback.IsNull ())
    {
      NS_FATAL_ERROR ("WirelessModuleUtility:Send packet callback is NOT set!");
    }

  // send jamming signal
  m_sendPacketCallback (packet, power, SEND_AS_HONEST);

  return power;
}

double
WirelessModuleUtility::DbmToW (double dBm) const
{
  double mW = pow(10.0,dBm/10.0);
  return mW / 1000.0;
}

double
WirelessModuleUtility::WToDbm (double w) const
{
  return 10.0 * log10(w * 1000.0);
}

} // namespace ns3
