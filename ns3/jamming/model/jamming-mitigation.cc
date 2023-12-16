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

#include "jamming-mitigation.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("JammingMitigation");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (JammingMitigation);

TypeId
JammingMitigation::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::JammingMitigation")
    .SetParent<Object> ()
    ;
  return tid;
}

JammingMitigation::JammingMitigation ()
  :  m_mitigationOn (false) // turn off mitigation by default
{
}

JammingMitigation::~JammingMitigation ()
{
}

void
JammingMitigation::SetId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_id = id;
}

uint32_t
JammingMitigation::GetId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_id;
}

void
JammingMitigation::StartMitigation (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("At Node #" << m_id << ", Mitigation of jamming started at " <<
                Simulator::Now ().GetSeconds() << "s");
  m_mitigationOn = true;  // set flag
  DoMitigation ();
}

void
JammingMitigation::StopMitigation (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("At Node #" << m_id << ", Mitigation of jamming stopped at " <<
                Simulator::Now ().GetSeconds() << "s");
  m_mitigationOn = false;  // set flag
  DoStopMitigation ();
}

bool
JammingMitigation::StartRxHandler (Ptr<Packet> packet, double startRss)
{
  NS_LOG_FUNCTION (this << packet << startRss);
  if (m_mitigationOn)
    {
      DoStartRxHandler (packet, startRss);
    }
  else
    {
      NS_LOG_DEBUG ("At Node #" << m_id << ", Mitigation of jamming is OFF!");
    }
  return true;
}

bool
JammingMitigation::EndRxHandler (Ptr<Packet> packet, double averageRss)
{
  NS_LOG_FUNCTION (this << packet << averageRss);
  if (m_mitigationOn)
  {
      DoEndRxHandler (packet, averageRss);
    }
  else
    {
      NS_LOG_DEBUG ("At Node #" << m_id << ", !");
      Synchronisation(packet, averageRss);
    }
  return true;
}

void
JammingMitigation::EndTxHandler (Ptr<Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);
  if (m_mitigationOn)
    {
      DoEndTxHandler (packet, txPower);
    }
  else
    {
      NS_LOG_DEBUG ("At Node #" << m_id <<
                    ", Mitigation of jamming is OFF, ignoring end TX event!");
    }
}

/*
 * Protected functions start here.
 */

bool
JammingMitigation::IsMitigationOn (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mitigationOn;
}

} // namespace ns3