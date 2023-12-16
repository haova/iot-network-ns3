/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

#include "detection-container.h"
#include "ns3/names.h"

namespace ns3 {

DetectionContainer::DetectionContainer ()
{
}

DetectionContainer::DetectionContainer (Ptr<Detection> mitigation)
{
  NS_ASSERT (mitigation != NULL);
  m_mitigators.push_back (mitigation);
}

DetectionContainer::DetectionContainer (std::string mitigationName)
{
  Ptr<Detection> mitigation = Names::Find<Detection> (mitigationName);
  NS_ASSERT (mitigation != NULL);
  m_mitigators.push_back (mitigation);
}

DetectionContainer::DetectionContainer (const DetectionContainer &a,
                                                        const DetectionContainer &b)
{
  *this = a;
  Add (b);
}

DetectionContainer::Iterator
DetectionContainer::Begin (void) const
{
  return m_mitigators.begin ();
}

DetectionContainer::Iterator
DetectionContainer::End (void) const
{
  return m_mitigators.end ();
}

uint32_t
DetectionContainer::GetN (void) const
{
  return m_mitigators.size ();
}

Ptr<Detection>
DetectionContainer::Get (uint32_t i) const
{
  return m_mitigators[i];
}

void
DetectionContainer::Add (DetectionContainer container)
{
  for (Iterator i = container.Begin (); i != container.End (); i++)
    {
      m_mitigators.push_back (*i);
    }
}

void
DetectionContainer::Add (Ptr<Detection> mitigation)
{
  NS_ASSERT (mitigation != NULL);
  m_mitigators.push_back (mitigation);
}

void
DetectionContainer::Add (std::string mitigationName)
{
  Ptr<Detection> mitigation = Names::Find<Detection> (mitigationName);
  NS_ASSERT (mitigation != NULL);
  m_mitigators.push_back (mitigation);
}

} // namespace ns3
