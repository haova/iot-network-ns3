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

#include "jammer.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("Jammer");

/*
 * Jammer base class.
 */
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Jammer);

TypeId
Jammer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Jammer")
    .SetParent<Object> ()
    ;
  return tid;
}

Jammer::Jammer (void)
  :  m_jammerOn (false) // jammer off by default
{
}

Jammer::~Jammer (void)
{
}

void
Jammer::SetId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_id = id;
}

uint32_t
Jammer::GetId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_id;
}

void
Jammer::StartJammer (void)
{
  NS_LOG_FUNCTION (this);
  m_jammerOn = true;  // turn jammer on
  DoJamming ();       // call jamming function
}

void
Jammer::StopJammer (void)
{
  NS_LOG_FUNCTION (this);
  m_jammerOn = false; // turn jammer off
  DoStopJammer ();    // stop jammer
}

bool
Jammer::StartRxHandler (Ptr<Packet> packet, double startRss)
{
  NS_LOG_FUNCTION (this << packet << startRss);
  NS_LOG_FUNCTION (this << packet->GetSize());
  
  if (m_jammerOn)
    {
      return DoStartRxHandler (packet, startRss);
    }
  else
    {
      NS_LOG_DEBUG ("At Node #" << m_id << ", Jammer is OFF, ignoring StartRx!");
      return false; // when jammer is off, all incoming packets are ignored.
    }
}

bool
Jammer::EndRxHandler (Ptr<Packet> packet, double averageRss)
{
  NS_LOG_FUNCTION (this << packet << averageRss);
  if (m_jammerOn)
    {
      return DoEndRxHandler (packet, averageRss);
    }
  else
    {
      NS_LOG_DEBUG ("At Node #" << m_id << ", Jammer is OFF, ignoring EndRx!");
      return false;
    }
}

void
Jammer::EndTxHandler (Ptr<Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);
  if (m_jammerOn)
    {
      DoEndTxHandler (packet, txPower);
    }
  else
    {
      NS_LOG_DEBUG ("At Node #" << m_id << ", Jammer is OFF, ignoring EndTx!");
    }
}


/*
 * Protected functions start here.
 */

bool
Jammer::IsJammerOn (void) const
{
  return m_jammerOn;
}

} // namespace ns3
