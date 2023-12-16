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

#include "jamming-mitigation-container.h"
#include "ns3/names.h"

namespace ns3 {

JammingMitigationContainer::JammingMitigationContainer ()
{
}

JammingMitigationContainer::JammingMitigationContainer (Ptr<JammingMitigation> mitigation)
{
  NS_ASSERT (mitigation != NULL);
  m_mitigators.push_back (mitigation);
}

JammingMitigationContainer::JammingMitigationContainer (std::string mitigationName)
{
  Ptr<JammingMitigation> mitigation = Names::Find<JammingMitigation> (mitigationName);
  NS_ASSERT (mitigation != NULL);
  m_mitigators.push_back (mitigation);
}

JammingMitigationContainer::JammingMitigationContainer (const JammingMitigationContainer &a,
                                                        const JammingMitigationContainer &b)
{
  *this = a;
  Add (b);
}

JammingMitigationContainer::Iterator
JammingMitigationContainer::Begin (void) const
{
  return m_mitigators.begin ();
}

JammingMitigationContainer::Iterator
JammingMitigationContainer::End (void) const
{
  return m_mitigators.end ();
}

uint32_t
JammingMitigationContainer::GetN (void) const
{
  return m_mitigators.size ();
}

Ptr<JammingMitigation>
JammingMitigationContainer::Get (uint32_t i) const
{
  return m_mitigators[i];
}

void
JammingMitigationContainer::Add (JammingMitigationContainer container)
{
  for (Iterator i = container.Begin (); i != container.End (); i++)
    {
      m_mitigators.push_back (*i);
    }
}

void
JammingMitigationContainer::Add (Ptr<JammingMitigation> mitigation)
{
  NS_ASSERT (mitigation != NULL);
  m_mitigators.push_back (mitigation);
}

void
JammingMitigationContainer::Add (std::string mitigationName)
{
  Ptr<JammingMitigation> mitigation = Names::Find<JammingMitigation> (mitigationName);
  NS_ASSERT (mitigation != NULL);
  m_mitigators.push_back (mitigation);
}

} // namespace ns3