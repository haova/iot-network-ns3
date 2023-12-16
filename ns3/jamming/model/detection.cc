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

#include "detection.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("Detection");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Detection);

TypeId
Detection::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Detection")
    .SetParent<Object> ()
    ;
  return tid;
}

Detection::Detection ()
  :  m_detectionOn (true) // turn off detection by default
{
}

Detection::~Detection ()
{
}

void
Detection::SetId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_id = id;
}

uint32_t
Detection::GetId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_id;
}

void
Detection::StartDetection (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("At Node #" << m_id << ", Detection of jamming started at " <<
                Simulator::Now ().GetSeconds() << "s");
  m_detectionOn = true;  // set flag
  DoDetection ();
}

void
Detection::StopDetection (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("At Node #" << m_id << ", Detection of jamming stopped at " <<
                Simulator::Now ().GetSeconds() << "s");
  m_detectionOn = false;  // set flag
  DoStopDetection ();
}

bool
Detection::StartRxHandler (Ptr<Packet> packet, double startRss)
{
  NS_LOG_FUNCTION (this << packet << startRss);
  if (m_detectionOn)
    {
      DoStartRxHandler (packet, startRss);
    }
  else
    {
      NS_LOG_DEBUG ("At Node #" << m_id << ", Detection of jamming is OFF!");
    }
  return true;
}

bool
Detection::EndRxHandler (Ptr<Packet> packet, double averageRss)
{
  NS_LOG_FUNCTION (this << packet << averageRss);
  if (m_detectionOn)
    {
      DoEndRxHandler (packet, averageRss);
    }
  else
    {
      NS_LOG_DEBUG ("At Node #" << m_id << ", Detection of jamming is OFF!");
    }
  return true;
}

void
Detection::EndTxHandler (Ptr<Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);
  if (m_detectionOn)
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
Detection::IsDetectionOn (void) const
{
  NS_LOG_FUNCTION (this);
  return m_detectionOn;
}

} // namespace ns3
