/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "interference-helper.h"
#include "wifi-phy.h"
#include "error-rate-model.h"
#include "wifi-utils.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("InterferenceHelper");

/****************************************************************
 *       Phy event class
 ****************************************************************/

Event::Event (Ptr<const Packet> packet, WifiTxVector txVector,Time duration, double rxPower)
  : m_packet (packet),
    m_txVector (txVector),
    m_startTime (Simulator::Now ()),
    m_endTime (m_startTime + duration),
    m_rxPowerW (rxPower)
{
}

Event::~Event ()
{
}

Ptr<const Packet>
Event::GetPacket (void) const
{
  return m_packet;
}

Time
Event::GetStartTime (void) const
{
  return m_startTime;
}

Time
Event::GetEndTime (void) const
{
  return m_endTime;
}

double
Event::GetRxPowerW (void) const
{
  return m_rxPowerW;
}

WifiTxVector
Event::GetTxVector (void) const
{
  return m_txVector;
}

WifiMode
Event::GetPayloadMode (void) const
{
  return m_txVector.GetMode ();
}
enum WifiPreamble
Event::GetPreambleType (void) const
{
  return m_preamble;
}



/****************************************************************
 *       Class which records SNIR change events for a
 *       short period of time.
 ****************************************************************/

InterferenceHelper::NiChange::NiChange (Time time, double delta)
  : m_time (time),
    m_delta (delta)
{
}


Time
InterferenceHelper::NiChange::GetTime (void) const
{
  return m_time;
}

double
InterferenceHelper::NiChange::GetDelta (void) const
{
  return m_delta;
}
bool
InterferenceHelper::NiChange::operator < (const InterferenceHelper::NiChange& o) const
{
  return (m_time < o.m_time);
}


double
InterferenceHelper::NiChange::GetPower (void) const
{
  return m_power;
}


void
InterferenceHelper::NiChange::AddPower (double power)
{
  m_power += power;
}

Ptr<Event>
InterferenceHelper::NiChange::GetEvent (void) const
{
  return m_event;
}


/****************************************************************
 *       The actual InterferenceHelper
 ****************************************************************/

InterferenceHelper::InterferenceHelper ()
  : m_errorRateModel (0),
    m_numRxAntennas (1),
    m_firstPower (0),
    m_rxing (false)
{
  // Always have a zero power noise event in the list
 // AddNiChangeEvent (Time (0), NiChange (0.0, 0));
}

InterferenceHelper::~InterferenceHelper ()
{
  EraseEvents ();
  m_errorRateModel = 0;
}

Ptr<Event>
InterferenceHelper::Add (Ptr<const Packet> packet, WifiTxVector txVector,Time duration, double rxPowerW)
{
  Ptr<Event> event = Create<Event> (packet, txVector, duration, rxPowerW);
  AppendEvent (event);
  return event;
}

void
InterferenceHelper::AddForeignSignal (Time duration, double rxPowerW)
{
  // Parameters other than duration and rxPowerW are unused for this type
  // of signal, so we provide dummy versions
  WifiTxVector fakeTxVector;
  Ptr<const Packet> packet (0);
  Add (packet, fakeTxVector, duration, rxPowerW);
}

void
InterferenceHelper::SetNoiseFigure (double value)
{
  m_noiseFigure = value;
}

double
InterferenceHelper::GetNoiseFigure (void) const
{
  return m_noiseFigure;
}

void
InterferenceHelper::SetErrorRateModel (const Ptr<ErrorRateModel> rate)
{
  m_errorRateModel = rate;
}

Ptr<ErrorRateModel>
InterferenceHelper::GetErrorRateModel (void) const
{
  return m_errorRateModel;
}

void
InterferenceHelper::SetNumberOfReceiveAntennas (uint8_t rx)
{
  m_numRxAntennas = rx;
}

Time
InterferenceHelper::GetEnergyDuration (double energyW) const
{
   Time now = Simulator::Now ();
   double noiseInterferenceW = 0.0;
     Time end = now;
     noiseInterferenceW = m_firstPower;
   for (NiChanges::const_iterator i = m_niChanges.begin (); i != m_niChanges.end (); i++)
       {
        noiseInterferenceW += i->GetDelta ();
         end = i->GetTime ();
         if (end < now)
           {
             continue;
           }
        if (noiseInterferenceW < energyW)
           {
             break;
           }
       }
   return end > now ? end - now : MicroSeconds (0);
}

void
InterferenceHelper::AppendEvent (Ptr<Event> event)
{
  NS_LOG_FUNCTION (this);
  /*double previousPowerStart = 0;
  //double previousPowerEnd = 0;
  previousPowerStart = GetPreviousPosition (event->GetStartTime ())->second.GetPower ();
  //previousPowerEnd = GetPreviousPosition (event->GetEndTime ())->second.GetPower ();
  NS_LOG_FUNCTION ("test pour voir");
  if (!m_rxing)
    {
      m_firstPower = previousPowerStart;
      // Always leave the first zero power noise event in the list
      m_niChanges.erase (++(m_niChanges.begin ()),
                         GetNextPosition (event->GetStartTime ()));
    }
  Time now = Simulator::Now ();
  auto first = AddNiChangeEvent (now,NiChange(event->GetStartTime (), event->GetRxPowerW ()));
  auto last = AddNiChangeEvent (now,NiChange(event->GetEndTime (), event->GetRxPowerW ()));
  for (auto i = first; i != last; ++i)
    {
      i->second.AddPower (event->GetRxPowerW ());
    }
    */
Time now = Simulator::Now ();
  if (!m_rxing)
     {
      NiChanges::iterator nowIterator = GetPosition (now);
       for (NiChanges::iterator i = m_niChanges.begin (); i != nowIterator; i++)
          {
            m_firstPower += i->GetDelta ();
           }
        m_niChanges.erase (m_niChanges.begin (), nowIterator);
        m_niChanges.insert (m_niChanges.begin (), NiChange (event->GetStartTime (), event->GetRxPowerW ()));
       }
    else
       {
         AddNiChangeEvent (NiChange (event->GetStartTime (), event->GetRxPowerW ()));
       }
     AddNiChangeEvent (NiChange (event->GetEndTime (), -event->GetRxPowerW ()));
  }

double
InterferenceHelper::CalculateSnr(double signal, double noiseInterference, WifiTxVector txVector) const
{
 /* //thermal noise at 290K in J/s = W
  static const double BOLTZMANN = 1.3803e-23;
  //Nt is the power of thermal noise in W
  double Nt = BOLTZMANN * 290 * channelWidth * 1e6;
  //receiver noise Floor (W) which accounts for thermal noise and non-idealities of the receiver
  double noiseFloor = m_noiseFigure * Nt;
  double noise = noiseFloor + noiseInterference;
  double snr = signal / noise; //linear scale
  NS_LOG_DEBUG ("bandwidth(MHz)=" << channelWidth << ", signal(W)= " << signal << ", noise(W)=" << noiseFloor << ", interference(W)=" << noiseInterference << ", snr=" << RatioToDb(snr) << "dB");
  return snr;*/

  double noiseFloor = CalculateNoiseFloor (txVector);
  double noise = noiseFloor + noiseInterference;
  double snr = signal / noise;
  return snr;
}

double
InterferenceHelper::CalculateSnr(Ptr<Event> event) const{
  WifiTxVector txvector= event->GetTxVector();
  double signal = event->GetRxPowerW ();
  double noiseInterference = m_firstPower;
  double noiseFloor = CalculateNoiseFloor(txvector);
  double noise = noiseFloor + noiseInterference;
  double snr = signal / noise;
  return snr;
}


  double
  InterferenceHelper::CalculateNoiseInterferenceW (Ptr<Event> event, NiChanges *ni) const
 {
   NS_LOG_INFO("noise1");
     double noiseInterference = m_firstPower;
    //NS_ASSERT (m_rxing);
    for (NiChanges::const_iterator i = m_niChanges.begin () + 1; i != m_niChanges.end (); i++)
       {
         NS_LOG_INFO("noise2" << event->GetEndTime());
         NS_LOG_INFO("noise2" << i->GetTime());
         NS_LOG_INFO("noise2" << event->GetRxPowerW());
         NS_LOG_INFO("noise2" << i->GetDelta());
       if ((event->GetEndTime () == i->GetTime()) && event->GetRxPowerW () == -i->GetDelta())
           {
            break;
          }
          NS_LOG_INFO("noise22");
        ni->push_back(*i);
        NS_LOG_INFO("noise221");
       }
       NS_LOG_INFO("noise3");
    ni->insert (ni->begin (), NiChange (event->GetStartTime (), noiseInterference));
    NS_LOG_INFO("noise4");
     ni->push_back (NiChange (event->GetEndTime (), 0));
     NS_LOG_INFO("noise4");
     return noiseInterference;
 }


double
InterferenceHelper::CalculateChunkSuccessRate (double snir, Time duration, WifiMode mode, WifiTxVector txVector) const
{
  if (duration == NanoSeconds (0))
      {
         return 1.0;
       }
     uint32_t rate = mode.GetPhyRate (txVector);
     uint64_t nbits = (uint64_t)(rate * duration.GetSeconds ());
     double csr = m_errorRateModel->GetChunkSuccessRate (mode, txVector, snir, nbits);
    return csr;
}

double
InterferenceHelper::CalculatePayloadPer (Ptr<const Event> event, NiChanges *ni, std::pair<Time, Time> window) const
{
  NS_LOG_FUNCTION (this << window.first << window.second);
  const WifiTxVector txVector = event->GetTxVector ();
  double psr = 1.0; /* Packet Success Rate */
  auto j = ni->begin ();
  Time previous = j->GetTime();
  WifiMode payloadMode = event->GetPayloadMode ();
  WifiPreamble preamble = txVector.GetPreambleType ();
  Time plcpHeaderStart = j->GetTime()+ WifiPhy::GetPlcpPreambleDuration (txVector); //packet start time + preamble
  Time plcpHsigHeaderStart = plcpHeaderStart + WifiPhy::GetPlcpHeaderDuration (txVector); //packet start time + preamble + L-SIG
  Time plcpTrainingSymbolsStart = plcpHsigHeaderStart + WifiPhy::GetPlcpHtSigHeaderDuration (preamble) + WifiPhy::GetPlcpSigA1Duration (preamble) + WifiPhy::GetPlcpSigA2Duration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A
  Time plcpPayloadStart = plcpTrainingSymbolsStart + WifiPhy::GetPlcpTrainingSymbolDuration (txVector) + WifiPhy::GetPlcpSigBDuration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A + Training + SIG-B
  Time windowStart = plcpPayloadStart + window.first;
  Time windowEnd = plcpPayloadStart + window.second;
  double noiseInterferenceW = m_firstPower;
  double powerW = event->GetRxPowerW ();
  while (++j != ni->end ())
    {
      Time current = j->GetTime();
      NS_LOG_DEBUG ("previous= " << previous << ", current=" << current);
      NS_ASSERT (current >= previous);
      //Case 1: Both previous and current point to the windowed payload
      if (previous >= windowStart)
        {
          psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                          noiseInterferenceW,
                                                          txVector),
                                            current - previous,
                                            payloadMode, txVector);
          NS_LOG_DEBUG ("Both previous and current point to the windowed payload: mode=" << payloadMode << ", psr=" << psr);
        }
      //Case 2: previous is before windowed payload and current is in the windowed payload
      else if (current >= windowStart)
        {
          psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                          noiseInterferenceW,
                                                          txVector),
                                            current - windowStart,
                                            payloadMode, txVector);
          NS_LOG_DEBUG ("previous is before windowed payload and current is in the windowed payload: mode=" << payloadMode << ", psr=" << psr);
        }
      noiseInterferenceW = j->GetPower () - powerW;
      previous = j->GetTime();
      if (previous > windowEnd)
        {
          NS_LOG_DEBUG ("Stop: new previous=" << previous << " after time window end=" << windowEnd);
          break;
        }

    }
  double per = 1 - psr;
  return per;
}

double
InterferenceHelper::CalculateNoiseFloor (WifiTxVector txVector) const
{
  // thermal noise at 290K in J/s = W
  static const double BOLTZMANN = 1.3803e-23;
  // Nt is the power of thermal noise in W
  double Nt = BOLTZMANN * 290.0 * txVector.GetChannelWidth();
  /*
   * Receiver noise Floor (W) which accounts for thermal noise and non-
   * idealities of the receiver.
   */
  return m_noiseFigure * Nt;
}

double
InterferenceHelper::CalculatePacketRss (Ptr<const Event> event,
                                        NiChanges *ni) const
{
  double rss = 0;
  double packetRss = 0;

  NiChanges::iterator j = ni->begin ();
  Time previous = j->GetTime ();
 // WifiMode payloadMode = event->GetPayloadMode ();
  //WifiPreamble preamble = event->GetPreambleType ();
  const WifiTxVector txVector = event->GetTxVector ();
  //WifiMode headerMode = WifiPhy::GetPlcpHeaderMode (txVector);
  Time plcpHeaderStart = j->GetTime () + MicroSeconds (WifiPhy::GetPlcpPreambleDuration (txVector));
  Time plcpPayloadStart = plcpHeaderStart + MicroSeconds (WifiPhy::GetPlcpHeaderDuration (txVector));
  double powerW = event->GetRxPowerW ();  // power of packet at receiver
  double noiseInterferenceW = j->GetDelta ();

  j++;
  while (ni->end () != j)
    {
      Time current = j->GetTime ();
      //SSERT (current >= previous);

      // payload only
      if (previous >= plcpPayloadStart)
        {
          rss = powerW + noiseInterferenceW + CalculateNoiseFloor (txVector);
          packetRss += rss * (current - previous).GetSeconds ();
        }
      else if (previous >= plcpHeaderStart)
        {
          // half header half payload
          if (current >= plcpPayloadStart)
            {
              // header chunk
              rss = powerW + noiseInterferenceW + CalculateNoiseFloor (txVector);
              packetRss += rss * (plcpPayloadStart - previous).GetSeconds ();
              // payload chunk
              rss = powerW + noiseInterferenceW + CalculateNoiseFloor (txVector);
              packetRss += rss * (current - plcpPayloadStart).GetSeconds ();
            }
          // header only
          else
            {
             NS_ASSERT (current >= plcpHeaderStart);
              rss = powerW + noiseInterferenceW + CalculateNoiseFloor (txVector);
              packetRss += rss * (current - previous).GetSeconds ();
            }
        }
      else
        {
          // half header half payload
          if (current >= plcpPayloadStart)
            {
              // header chunk
              rss = powerW + noiseInterferenceW + CalculateNoiseFloor (txVector);
              packetRss += rss * (plcpPayloadStart - plcpHeaderStart).GetSeconds ();
              // payload chunk
              rss = powerW + noiseInterferenceW + CalculateNoiseFloor (txVector);
              packetRss += rss * (current - plcpPayloadStart).GetSeconds ();
            }
          // header only
          else if (current >= plcpHeaderStart)
            {
              rss = powerW + noiseInterferenceW + CalculateNoiseFloor (txVector);
              packetRss += rss * (current - plcpHeaderStart).GetSeconds ();
            }
        }

      noiseInterferenceW += j->GetDelta ();
      previous = current;
      j++;
    }

  NS_ASSERT (event->GetEndTime().GetSeconds () != 0);
  // real duration = time stamp of (last ni change - start of header)
  Time duration = (ni->end()->GetTime () - plcpHeaderStart);
  packetRss /= duration.GetSeconds ();
  return packetRss;
}

double
InterferenceHelper::CurrentNodeRss (WifiTxVector mode)
{
  double rss = CalculateNoiseFloor (mode);
  Time now = Simulator::Now ();
  NiChanges::iterator nowIterator = GetPosition (now);
  for (NiChanges::iterator i = m_niChanges.begin (); i != nowIterator; i++)
    {
      rss += i->GetDelta ();
    }
  return rss;
}

double
InterferenceHelper::CalculatePer (Ptr<const Event> event, NiChanges *ni) const
{
  double psr = 1.0; /* Packet Success Rate */
  NiChanges::iterator j = ni->begin ();
  Time previous = (*j).GetTime();
  WifiMode payloadMode = event->GetPayloadMode ();
 // WifiPreamble preamble = event->GetPreambleType ();
  const WifiTxVector txVector = event->GetTxVector ();
  WifiMode headerMode = WifiPhy::GetPlcpHeaderMode (txVector);
  Time plcpHeaderStart = (*j).GetTime() + MicroSeconds (WifiPhy::GetPlcpPreambleDuration (txVector));
  Time plcpPayloadStart = plcpHeaderStart + MicroSeconds (WifiPhy::GetPlcpHeaderDuration (txVector));
  double noiseInterferenceW = (*j).GetDelta ();
  double powerW = event->GetRxPowerW ();

  j++;
  NS_LOG_DEBUG ("i am here avant ");
  while (ni->end () != j)
    {
      Time current = (*j).GetTime ();
      
      NS_LOG_DEBUG ("i am here");
      NS_LOG_DEBUG (current.GetDouble());
      NS_LOG_DEBUG ("test" <<previous.GetDouble());
      NS_LOG_DEBUG ("test resi" << current.Compare(previous));
      NS_ASSERT (current.Compare(previous) >=0);

      if (previous.Compare(plcpPayloadStart) == 1)
        {
          psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                          noiseInterferenceW,
                                                          txVector),
                                            current - previous,
                                            payloadMode,txVector);
        }
      else if (previous.Compare(plcpHeaderStart) >=0 )
        {
          if (current.Compare(plcpPayloadStart) >=0)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpPayloadStart - previous,
                                                headerMode,txVector);
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                current - plcpPayloadStart,
                                                payloadMode,txVector);
            }
          else
            {
              NS_ASSERT (current >= plcpHeaderStart);
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                current - previous,
                                                headerMode,txVector);
            }
        }
      else
        {
          if (current >= plcpPayloadStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpPayloadStart - plcpHeaderStart,
                                                headerMode,txVector);
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                current - plcpPayloadStart,
                                                payloadMode,txVector);
            }
          else if (current >= plcpHeaderStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                current - plcpHeaderStart,
                                                headerMode,txVector);
            }
        }

      noiseInterferenceW += (*j).GetDelta ();
      previous = (*j).GetTime ();
      j++;
    }

  double per = 1 - psr;
  return per;
}


struct InterferenceHelper::SnrPer
InterferenceHelper::CalculateSnrPer (Ptr<Event> event)
{
  NiChanges ni;
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni);
  double snr = CalculateSnr (event->GetRxPowerW (),
                             noiseInterferenceW,
                             event->GetTxVector());

  /*
   * calculate the SNIR at the start of the packet and accumulate
   * all SNIR changes in the snir vector.
   */
    NS_LOG_DEBUG ("ou ca ");
  double per = CalculatePer (event, &ni);

  struct SnrPer snrPer;
  snrPer.snr = snr;
  snrPer.per = per;
  snrPer.packetRss = CalculatePacketRss (event, &ni);

  return snrPer;
}

InterferenceHelper::NiChanges::iterator
InterferenceHelper::GetPosition (Time moment)
 {
     return std::upper_bound(m_niChanges.begin (), m_niChanges.end (), NiChange (moment, 0));
   
 }

double
InterferenceHelper::CalculateLegacyPhyHeaderPer (Ptr<const Event> event, NiChanges *ni) const
{
  NS_LOG_FUNCTION (this);
  const WifiTxVector txVector = event->GetTxVector ();
  double psr = 1.0; /* Packet Success Rate */
  auto j = ni->begin ();
  Time previous = j->GetTime();
  WifiPreamble preamble = txVector.GetPreambleType ();
  WifiMode headerMode = WifiPhy::GetPlcpHeaderMode (txVector);
  Time plcpHeaderStart = j->GetTime() + WifiPhy::GetPlcpPreambleDuration (txVector); //packet start time + preamble
  Time plcpHsigHeaderStart = plcpHeaderStart + WifiPhy::GetPlcpHeaderDuration (txVector); //packet start time + preamble + L-SIG
  Time plcpTrainingSymbolsStart = plcpHsigHeaderStart + WifiPhy::GetPlcpHtSigHeaderDuration (preamble) + WifiPhy::GetPlcpSigA1Duration (preamble) + WifiPhy::GetPlcpSigA2Duration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A
  Time plcpPayloadStart = plcpTrainingSymbolsStart + WifiPhy::GetPlcpTrainingSymbolDuration (txVector) + WifiPhy::GetPlcpSigBDuration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A + Training + SIG-B
  double noiseInterferenceW = m_firstPower;
  double powerW = event->GetRxPowerW ();
  while (++j != ni->end ())
    {
      Time current = j->GetTime();
      NS_LOG_DEBUG ("previous= " << previous << ", current=" << current);
      NS_ASSERT (current >= previous);
      //Case 1: previous and current after playload start
      if (previous >= plcpPayloadStart)
        {
          psr *= 1;
          NS_LOG_DEBUG ("Case 1 - previous and current after playload start: nothing to do");
        }
      //Case 2: previous is in training or in SIG-B: legacy will not enter here since it didn't enter in the last two and they are all the same for legacy
      else if (previous >= plcpTrainingSymbolsStart)
        {
         NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
          psr *= 1;
          NS_LOG_DEBUG ("Case 2 - previous is in training or in SIG-B: nothing to do");
        }
      //Case 3: previous is in HT-SIG or SIG-A: legacy will not enter here since it didn't enter in the last two and they are all the same for legacy
      else if (previous >= plcpHsigHeaderStart)
        {
         NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
          psr *= 1;
          NS_LOG_DEBUG ("Case 3cii - previous is in HT-SIG or SIG-A: nothing to do");
        }
      //Case 4: previous in L-SIG: HT GF will not reach here because it will execute the previous if and exit
      else if (previous >= plcpHeaderStart)
        {
         NS_ASSERT (preamble != WIFI_PREAMBLE_HT_GF);
          //Case 4a: current after payload start
          if (current >= plcpPayloadStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpHsigHeaderStart - previous,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 4a - previous in L-SIG and current after payload start: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 4b: current is in training or in SIG-B. legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpTrainingSymbolsStart)
            {
             NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpHsigHeaderStart - previous,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 4a - previous in L-SIG and current is in training or in SIG-B: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 4c: current in HT-SIG or in SIG-A. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpHsigHeaderStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpHsigHeaderStart - previous,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 4ci - previous is in L-SIG and current in HT-SIG or in SIG-A: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 4d: current with previous in L-SIG
          else
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                current - previous,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 4d - current with previous in L-SIG: mode=" << headerMode << ", psr=" << psr);
            }
        }
      //Case 5: previous is in the preamble works for all cases
      else
        {
          //Case 5a: current after payload start
          if (current >= plcpPayloadStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpHsigHeaderStart - plcpHeaderStart,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 5aii - previous is in the preamble and current is after payload start: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 5b: current is in training or in SIG-B. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpTrainingSymbolsStart)
            {
             NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpHsigHeaderStart - plcpHeaderStart,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 5b - previous is in the preamble and current is in training or in SIG-B: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 5c: current in HT-SIG or in SIG-A. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpHsigHeaderStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpHsigHeaderStart - plcpHeaderStart,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 5b - previous is in the preamble and current in HT-SIG or in SIG-A: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 5d: current is in L-SIG.
          else if (current >= plcpHeaderStart)
            {
             NS_ASSERT (preamble != WIFI_PREAMBLE_HT_GF);
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                current - plcpHeaderStart,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 5d - previous is in the preamble and current is in L-SIG: mode=" << headerMode << ", psr=" << psr);
            }
        }

      noiseInterferenceW = j->GetPower () - powerW;
      previous = j->GetTime();
    }

  double per = 1 - psr;
  return per;
}

double
InterferenceHelper::CalculateNonLegacyPhyHeaderPer (Ptr<const Event> event, NiChanges *ni) const
{
  NS_LOG_FUNCTION (this);
  const WifiTxVector txVector = event->GetTxVector ();
  double psr = 1.0; /* Packet Success Rate */
  auto j = ni->begin ();
  Time previous = j->GetTime();
  WifiPreamble preamble = txVector.GetPreambleType ();
  WifiMode mcsHeaderMode;
  if (preamble == WIFI_PREAMBLE_HT_MF || preamble == WIFI_PREAMBLE_HT_GF)
    {
      //mode for PLCP header fields sent with HT modulation
      mcsHeaderMode = WifiPhy::GetHtPlcpHeaderMode ();
    }
  else if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_VHT_MU)
    {
      //mode for PLCP header fields sent with VHT modulation
      mcsHeaderMode = WifiPhy::GetVhtPlcpHeaderMode ();
    }
  else if (preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_HE_MU)
    {
      //mode for PLCP header fields sent with HE modulation
      mcsHeaderMode = WifiPhy::GetHePlcpHeaderMode ();
    }
  WifiMode headerMode = WifiPhy::GetPlcpHeaderMode (txVector);
  Time plcpHeaderStart = j->GetTime() + WifiPhy::GetPlcpPreambleDuration (txVector); //packet start time + preamble
  Time plcpHsigHeaderStart = plcpHeaderStart + WifiPhy::GetPlcpHeaderDuration (txVector); //packet start time + preamble + L-SIG
  Time plcpTrainingSymbolsStart = plcpHsigHeaderStart + WifiPhy::GetPlcpHtSigHeaderDuration (preamble) + WifiPhy::GetPlcpSigA1Duration (preamble) + WifiPhy::GetPlcpSigA2Duration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A
  Time plcpPayloadStart = plcpTrainingSymbolsStart + WifiPhy::GetPlcpTrainingSymbolDuration (txVector) + WifiPhy::GetPlcpSigBDuration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A + Training + SIG-B
  double noiseInterferenceW = m_firstPower;
  double powerW = event->GetRxPowerW ();
  while (++j != ni->end ())
    {
      Time current = j->GetTime();
      NS_LOG_DEBUG ("previous= " << previous << ", current=" << current);
      NS_ASSERT (current >= previous);
      //Case 1: previous and current after playload start: nothing to do
      if (previous >= plcpPayloadStart)
        {
          psr *= 1;
          NS_LOG_DEBUG ("Case 1 - previous and current after playload start: nothing to do");
        }
      //Case 2: previous is in training or in SIG-B: legacy will not enter here since it didn't enter in the last two and they are all the same for legacy
      else if (previous >= plcpTrainingSymbolsStart)
        {
         NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
          //Case 2a: current after payload start
          if (current >= plcpPayloadStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpPayloadStart - previous,
                                                mcsHeaderMode, txVector);
              NS_LOG_DEBUG ("Case 2a - previous is in training or in SIG-B and current after payload start: mode=" << mcsHeaderMode << ", psr=" << psr);
            }
          //Case 2b: current is in training or in SIG-B
          else
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                current - previous,
                                                mcsHeaderMode, txVector);
              NS_LOG_DEBUG ("Case 2b - previous is in training or in SIG-B and current is in training or in SIG-B: mode=" << mcsHeaderMode << ", psr=" << psr);
            }
        }
      //Case 3: previous is in HT-SIG or SIG-A: legacy will not enter here since it didn't enter in the last two and they are all the same for legacy
      else if (previous >= plcpHsigHeaderStart)
        {
         NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
          //Case 3a: current after payload start
          if (current >= plcpPayloadStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                plcpPayloadStart - plcpTrainingSymbolsStart,
                                                mcsHeaderMode, txVector);
              //Case 3ai: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  //SIG-A is sent using legacy OFDM modulation
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpTrainingSymbolsStart - previous,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 3ai - previous is in SIG-A and current after payload start: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 3aii: HT formats
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpTrainingSymbolsStart - previous,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 3aii - previous is in HT-SIG and current after payload start: mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 3b: current is in training or in SIG-B
          else if (current >= plcpTrainingSymbolsStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector),
                                                current - plcpTrainingSymbolsStart,
                                                mcsHeaderMode, txVector);
              //Case 3bi: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  //SIG-A is sent using legacy OFDM modulation
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpTrainingSymbolsStart - previous,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 3bi - previous is in SIG-A and current is in training or in SIG-B: mode=" << headerMode << ", psr=" << psr);
                }
              //Case 3bii: HT formats
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpTrainingSymbolsStart - previous,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 3bii - previous is in HT-SIG and current is in HT training: mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 3c: current with previous in HT-SIG or SIG-A
          else
            {
              //Case 3ci: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  //SIG-A is sent using legacy OFDM modulation
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - previous,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 3ci - previous with current in SIG-A: mode=" << headerMode << ", psr=" << psr);
                }
              //Case 3cii: HT mixed format or HT greenfield
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - previous,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 3cii - previous with current in HT-SIG: mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
        }
      //Case 4: previous in L-SIG: HT GF will not reach here because it will execute the previous if and exit
      else if (previous >= plcpHeaderStart)
        {
          NS_ASSERT (preamble != WIFI_PREAMBLE_HT_GF);
          //Case 4a: current after payload start
          if (current >= plcpPayloadStart)
            {
              //Case 4ai: legacy format
              if (preamble == WIFI_PREAMBLE_LONG || preamble == WIFI_PREAMBLE_SHORT)
                {
                  psr *= 1;
                  NS_LOG_DEBUG ("Case 4ai - previous in L-SIG and current after payload start: nothing to do");
                }
              //Case 4aii: VHT or HE format
              else if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpPayloadStart - plcpTrainingSymbolsStart,
                                                    mcsHeaderMode, txVector);
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpTrainingSymbolsStart - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 4aii - previous is in L-SIG and current after payload start: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 4aiii: HT mixed format
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpPayloadStart - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 4aiii - previous in L-SIG and current after payload start: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 4b: current is in training or in SIG-B. legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpTrainingSymbolsStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              //Case 4bi: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - plcpTrainingSymbolsStart,
                                                    mcsHeaderMode, txVector);
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpTrainingSymbolsStart - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 4bi - previous is in L-SIG and current in training or in SIG-B: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 4bii: HT mixed format
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 4bii - previous in L-SIG and current in HT training: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 4c: current in HT-SIG or in SIG-A. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpHsigHeaderStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              //Case 4ci: VHT format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 4ci - previous is in L-SIG and current in SIG-A: mode=" << headerMode << ", psr=" << psr);
                }
              //Case 4cii: HT mixed format
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 4cii - previous in L-SIG and current in HT-SIG: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 4d: current with previous in L-SIG
          else
            {
              psr *= 1;
              NS_LOG_DEBUG ("Case 4d - current with previous in L-SIG: nothing to do");
            }
        }
      //Case 5: previous is in the preamble works for all cases
      else
        {
          //Case 5a: current after payload start
          if (current >= plcpPayloadStart)
            {
              //Case 5ai: legacy format (No HT-SIG or Training Symbols)
              if (preamble == WIFI_PREAMBLE_LONG || preamble == WIFI_PREAMBLE_SHORT)
                {
                  psr *= 1;
                  NS_LOG_DEBUG ("Case 5ai - previous is in the preamble and current is after payload start: nothing to do");
                }
              //Case 5aii: VHT or HE format
              else if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpPayloadStart - plcpTrainingSymbolsStart,
                                                    mcsHeaderMode, txVector);
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpTrainingSymbolsStart - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 5aii - previous is in the preamble and current is after payload start: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 5aiii: HT formats
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpPayloadStart - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 5aiii - previous is in the preamble and current is after payload start: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
            }
          //Case 5b: current is in training or in SIG-B. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpTrainingSymbolsStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              //Case 5bi: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - plcpTrainingSymbolsStart,
                                                    mcsHeaderMode, txVector);
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    plcpTrainingSymbolsStart - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 5bi - previous is in the preamble and current in training or in SIG-B: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 5bii: HT mixed format
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 5bii - previous is in the preamble and current in HT training: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 5c: current in HT-SIG or in SIG-A. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpHsigHeaderStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              //Case 5ci: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 5ci - previous is in preamble and current in SIG-A: mode=" << headerMode << ", psr=" << psr);
                }
              //Case 5cii: HT formats
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector),
                                                    current - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 5cii - previous in preamble and current in HT-SIG: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 5d: current is in L-SIG. HT-GF will not come here
          else if (current >= plcpHeaderStart)
            {
             NS_ASSERT (preamble != WIFI_PREAMBLE_HT_GF);
              psr *= 1;
              NS_LOG_DEBUG ("Case 5d - previous is in the preamble and current is in L-SIG: nothing to do");
            }
        }

      noiseInterferenceW = j->GetPower () - powerW;
      previous = j->GetTime();
    }

  double per = 1 - psr;
  return per;
}

struct InterferenceHelper::SnrPer
InterferenceHelper::CalculatePayloadSnrPer (Ptr<Event> event, std::pair<Time, Time> relativeMpduStartStop) const
{
  NiChanges ni;
  NS_LOG_INFO("interference1");
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni);
  NS_LOG_INFO("interference2");
  double snr = CalculateSnr (event->GetRxPowerW (),
                             noiseInterferenceW,
                             event->GetTxVector ());
  NS_LOG_INFO("interference3");

  /* calculate the SNIR at the start of the MPDU (located through windowing) and accumulate
   * all SNIR changes in the snir vector.
   */
  double per = CalculatePayloadPer (event, &ni, relativeMpduStartStop);

  struct SnrPer snrPer;
  snrPer.snr = snr;
  snrPer.per = per;
  return snrPer;
}



struct InterferenceHelper::SnrPer
InterferenceHelper::CalculateLegacyPhyHeaderSnrPer (Ptr<Event> event) const
{
  NiChanges ni;
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni);
  double snr = CalculateSnr (event->GetRxPowerW (),
                             noiseInterferenceW,
                             event->GetTxVector ());

  /* calculate the SNIR at the start of the plcp header and accumulate
   * all SNIR changes in the snir vector.
   */
  double per = CalculateLegacyPhyHeaderPer (event, &ni);

  struct SnrPer snrPer;
  snrPer.snr = snr;
  snrPer.per = per;
  return snrPer;
}

struct InterferenceHelper::SnrPer
InterferenceHelper::CalculateNonLegacyPhyHeaderSnrPer (Ptr<Event> event) const
{
  NiChanges ni;
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni);
  double snr = CalculateSnr (event->GetRxPowerW (),
                             noiseInterferenceW,
                             event->GetTxVector());
  
  /* calculate the SNIR at the start of the plcp header and accumulate
   * all SNIR changes in the snir vector.
   */
  double per = CalculateNonLegacyPhyHeaderPer (event, &ni);
  
  struct SnrPer snrPer;
  snrPer.snr = snr;
  snrPer.per = per;
  return snrPer;
}

void
InterferenceHelper::EraseEvents (void)
{
  m_niChanges.clear ();
  // Always have a zero power noise event in the list
  AddNiChangeEvent (NiChange (Time(0.0), 0));
  m_rxing = false;
  m_firstPower = 0;
}

InterferenceHelper::NiChanges::const_iterator
InterferenceHelper::GetNextPosition (Time moment) const
{
   return std::upper_bound(m_niChanges.begin (), m_niChanges.end (), NiChange (moment, 0));
   
}

InterferenceHelper::NiChanges::const_iterator
InterferenceHelper::GetPreviousPosition (Time moment) const
{
  auto it = GetNextPosition (moment);
  // This is safe since there is always an NiChange at time 0,
  // before moment.
  --it;
  return it;
}

void
InterferenceHelper::AddNiChangeEvent ( NiChange change)
{
  m_niChanges.insert (GetPosition (change.GetTime ()), change);
}

void
InterferenceHelper::NotifyRxStart ()
{
  NS_LOG_FUNCTION (this);
  m_rxing = true;
}

void
InterferenceHelper::NotifyRxEnd ()
{
  NS_LOG_FUNCTION (this);
  m_rxing = false;
  //Update m_firstPower for frame capture
  /*auto it = m_niChanges.find (Simulator::Now ());
  it--;
  m_firstPower = it->second.GetPower ();*/
}

} //namespace ns3
