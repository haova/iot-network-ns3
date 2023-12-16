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
 * Author: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

#include "eavesdropper-jammer.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE ("EavesdropperJammer");

/*
 * Eavesdropper Jammer.
 */
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (EavesdropperJammer);

TypeId
EavesdropperJammer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EavesdropperJammer")
    .SetParent<Jammer> ()
    .AddConstructor<EavesdropperJammer> ()
    .AddAttribute ("EavesdropperJammerRxTimeout",
                   "Rx timeout.",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&EavesdropperJammer::SetRxTimeout,
                                     &EavesdropperJammer::GetRxTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("EavesdropperJammerScanMode",
                   "Whether to run in scan mode.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&EavesdropperJammer::SetScanMode,
                                         &EavesdropperJammer::GetScanMode),
                   MakeUintegerChecker<bool> ())
    .AddAttribute ("EavesdropperJammerNumOfScanCycle",
                   "Eavesdropper jammer scan cycles.",
                   UintegerValue (2),
                   MakeUintegerAccessor (&EavesdropperJammer::SetScanCycles,
                                         &EavesdropperJammer::GetScanCycles),
                   MakeUintegerChecker<uint32_t> ())
    ;
  return tid;
}

EavesdropperJammer::EavesdropperJammer ()
  :  m_scanCount (0),
     m_scanComplete (false),
     m_isScan (false)
{
  m_numOfPktsReceived.clear (); // clear packet count list
}

EavesdropperJammer::~EavesdropperJammer ()
{
}

void
EavesdropperJammer::SetUtility (Ptr<WirelessModuleUtility> utility)
{
  NS_LOG_FUNCTION (this << utility);
  NS_ASSERT (utility != NULL);
  m_utility = utility;
}

void
EavesdropperJammer::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}

void
EavesdropperJammer::SetRxTimeout (Time timeout)
{
  NS_LOG_FUNCTION (this << timeout);
  m_rxTimeout = timeout;
}

Time
EavesdropperJammer::GetRxTimeout (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxTimeout;
}

void
EavesdropperJammer::SetScanMode (bool mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_isScan = mode;
}

bool
EavesdropperJammer::GetScanMode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_isScan;
}

void
EavesdropperJammer::SetScanCycles (uint32_t cycles)
{
  NS_LOG_FUNCTION (this << cycles);
  m_totalScanCycles = cycles;
}

uint32_t
EavesdropperJammer::GetScanCycles (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalScanCycles;
}

void
EavesdropperJammer::ClearPacketCountList (void)
{
  NS_LOG_FUNCTION (this);
  m_numOfPktsReceived.clear ();
}

/*
 * Private functions start here.
 */

void
EavesdropperJammer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}

void
EavesdropperJammer::DoStopJammer (void)
{
  NS_LOG_FUNCTION (this);
  m_rxTimeoutEvent.Cancel ();
}

void
EavesdropperJammer::DoJamming (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("EavesdropperJammer:At Node #" << GetId () <<
                ", Scheduling initial RX timeout at DoJamming");

  if (m_isScan) // check if in scan mode
    {
      m_rxTimeoutEvent.Cancel (); // cancel previous RX timeout
      // schedule RX timeout
      m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                              &EavesdropperJammer::RxTimeoutHandler,
                                              this);
    }
}

bool
EavesdropperJammer::DoStartRxHandler (Ptr<Packet> packet, double startRss)
{
  NS_LOG_FUNCTION (this << packet << startRss);
  return true;
}

bool
EavesdropperJammer::DoEndRxHandler (Ptr<Packet> packet, double averageRss)
{
  NS_LOG_FUNCTION (this << packet << averageRss);
  NS_LOG_DEBUG ("EavesdropperJammer:At Node #" << GetId () << ", Handling packet!");

  if (!IsJammerOn ())
    {
      NS_LOG_DEBUG ("EavesdropperJammer:At Node #" << GetId () << ", Jammer is OFF!");
      return false;
    }

  if (m_isScan)
    {
      // initialize packet count list ONLY if it's empty.
      if (m_numOfPktsReceived.empty ())
        {
          m_numOfPktsReceived.assign (m_utility->GetPhyLayerInfo ().numOfChannels,
                                      false);
        }
      if (!m_scanComplete)
        {
          // check if packet is valid, NULL means receive failed
          if (packet != NULL)
            {
              uint16_t currentChannelNumber = m_utility->GetPhyLayerInfo ().currentChannel;
              m_numOfPktsReceived[currentChannelNumber]++;
            }
        }
      return true;
    }

  // TODO non-scan mode is to be implemented in future releases.
  return true;
}

void
EavesdropperJammer::DoEndTxHandler (Ptr<Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);
  // we should never reach this function, because eavesdropper never transmits.
  NS_FATAL_ERROR ("EavesdropperJammer:At Node #" << GetId () <<
                  ", DoEndTxHandler Called!");
}

void
EavesdropperJammer::RxTimeoutHandler (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("EavesdropperJammer:At Node #" << GetId () <<
                ", RX timeout occurred!");

  m_rxTimeoutEvent.Cancel (); // cancel previously scheduled event

  /*
   * RX timeout is always scheduled so that if a jammer is stopped during scan,
   * it can be resumed. When jammer is turned off or scan cycles are completed,
   * no more channel switch will be performed.
   */

  if (Simulator::IsFinished ()) // do not reschedule when simulation ends
    {
      return;
    }

  // switch to next channel
  if (IsJammerOn () && !m_scanComplete)
    {
      // check to see if we need t restart from next channel.
      uint16_t channelNumber = m_utility->GetPhyLayerInfo ().currentChannel;
      if (++channelNumber >= m_utility->GetPhyLayerInfo ().numOfChannels + 1)
        {
          channelNumber = 1;  // wrap around
        }
      NS_LOG_DEBUG ("EavesdropperJammer:At Node #" << GetId () <<
                    ", Switching channel from " <<
                    m_utility->GetPhyLayerInfo ().currentChannel << " >-> " <<
                    channelNumber);
      m_utility->SwitchChannel (channelNumber);
      m_scanCount++;  // increase scan count
    }

  // check if scan is complete
  if (m_scanCount / m_utility->GetPhyLayerInfo ().numOfChannels >=
      m_totalScanCycles)
    {
      m_scanComplete = true;
      NS_LOG_DEBUG ("EavesdropperJammer:At Node #" << GetId () <<
                    ", Scan completed!");
    }

  // schedule next RX timeout
  m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                          &EavesdropperJammer::RxTimeoutHandler,
                                          this);
}

} // namespace ns3
