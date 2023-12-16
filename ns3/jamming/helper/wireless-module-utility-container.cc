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

#include "wireless-module-utility-container.h"
#include "ns3/names.h"

namespace ns3 {

WirelessModuleUtilityContainer::WirelessModuleUtilityContainer ()
{
}

WirelessModuleUtilityContainer::WirelessModuleUtilityContainer (Ptr<WirelessModuleUtility> utility)
{
  NS_ASSERT (utility != NULL);
  m_utilities.push_back (utility);
}

WirelessModuleUtilityContainer::WirelessModuleUtilityContainer (std::string utilityName)
{
  Ptr<WirelessModuleUtility> utility = Names::Find<WirelessModuleUtility> (utilityName);
  NS_ASSERT (utility != NULL);
  m_utilities.push_back (utility);
}

WirelessModuleUtilityContainer::WirelessModuleUtilityContainer (const WirelessModuleUtilityContainer &a,
                                                                const WirelessModuleUtilityContainer &b)
{
  *this = a;
  Add (b);
}

WirelessModuleUtilityContainer::Iterator
WirelessModuleUtilityContainer::Begin (void) const
{
  return m_utilities.begin ();
}

WirelessModuleUtilityContainer::Iterator
WirelessModuleUtilityContainer::End (void) const
{
  return m_utilities.end ();
}

uint32_t
WirelessModuleUtilityContainer::GetN (void) const
{
  return m_utilities.size ();
}

Ptr<WirelessModuleUtility>
WirelessModuleUtilityContainer::Get (uint32_t i) const
{
  return m_utilities[i];
}

void
WirelessModuleUtilityContainer::Add (WirelessModuleUtilityContainer container)
{
  for (Iterator i = container.Begin (); i != container.End (); i++)
    {
      m_utilities.push_back (*i);
    }
}

void
WirelessModuleUtilityContainer::Add (Ptr<WirelessModuleUtility> utility)
{
  NS_ASSERT (utility != NULL);
  m_utilities.push_back (utility);
}

void
WirelessModuleUtilityContainer::Add (std::string utilityName)
{
  Ptr<WirelessModuleUtility> utility = Names::Find<WirelessModuleUtility> (utilityName);
  NS_ASSERT (utility != NULL);
  m_utilities.push_back (utility);
}

} // namespace ns3
