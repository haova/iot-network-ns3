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

#include "jammer-container.h"
#include "ns3/names.h"

namespace ns3 {

JammerContainer::JammerContainer ()
{
}

JammerContainer::JammerContainer (Ptr<Jammer> jammer)
{
  NS_ASSERT (jammer != NULL);
  m_jammers.push_back (jammer);
}

JammerContainer::JammerContainer (std::string jammerName)
{
  Ptr<Jammer> jammer = Names::Find<Jammer> (jammerName);
  NS_ASSERT (jammer != NULL);
  m_jammers.push_back (jammer);
}

JammerContainer::JammerContainer (const JammerContainer &a,
                                  const JammerContainer &b)
{
  *this = a;
  Add (b);
}

JammerContainer::Iterator
JammerContainer::Begin (void) const
{
  return m_jammers.begin ();
}

JammerContainer::Iterator
JammerContainer::End (void) const
{
  return m_jammers.end ();
}

uint32_t
JammerContainer::GetN (void) const
{
  return m_jammers.size ();
}

Ptr<Jammer>
JammerContainer::Get (uint32_t i) const
{
  return m_jammers[i];
}

void
JammerContainer::Add (JammerContainer container)
{
  for (Iterator i = container.Begin (); i != container.End (); i++)
    {
      m_jammers.push_back (*i);
    }
}

void
JammerContainer::Add (Ptr<Jammer> jammer)
{
  NS_ASSERT (jammer != NULL);
  m_jammers.push_back (jammer);
}

void
JammerContainer::Add (std::string jammerName)
{
  Ptr<Jammer> jammer = Names::Find<Jammer> (jammerName);
  NS_ASSERT (jammer != NULL);
  m_jammers.push_back (jammer);
}

} // namespace ns3
