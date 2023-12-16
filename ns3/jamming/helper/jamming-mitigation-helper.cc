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

#include "jamming-mitigation-helper.h"
#include "ns3/energy-source-container.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/names.h"

NS_LOG_COMPONENT_DEFINE ("JammingMitigationHelper");

namespace ns3 {

JammingMitigationHelper::JammingMitigationHelper ()
{
  m_jammingMitigation.SetTypeId ("ns3::MitigateByChannelHop");
}

JammingMitigationHelper::~JammingMitigationHelper ()
{
}

void
JammingMitigationHelper::SetJammingMitigationType (std::string type)
{
  NS_LOG_FUNCTION (this);
  m_jammingMitigation.SetTypeId (type);
}

std::string
JammingMitigationHelper::GetJammingMitigationType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_jammingMitigation.GetTypeId ().GetName ();
}

void
JammingMitigationHelper::Set (std::string name, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_jammingMitigation.Set (name, v);
}

JammingMitigationContainer
JammingMitigationHelper::Install (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  return Install (NodeContainer (node));
}

JammingMitigationContainer
JammingMitigationHelper::Install (NodeContainer c) const
{
  NS_LOG_DEBUG (this);
  JammingMitigationContainer container;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<JammingMitigation> mitigator = DoInstall (*i);
      container.Add (mitigator);
    }
  return container;
}

JammingMitigationContainer
JammingMitigationHelper::Install (std::string nodeName) const
{
  NS_LOG_DEBUG (this);
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (node);
}

void
JammingMitigationHelper::EnableLogComponents (void)
{
  LogComponentEnable ("JammingMitigation", LOG_LEVEL_ALL);
  LogComponentEnable ("MitigateByChannelHop", LOG_LEVEL_ALL);
}

/*
 * Private function starts here.
 */

Ptr<JammingMitigation>
JammingMitigationHelper::DoInstall (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  NS_LOG_DEBUG ("JammingMitigationHelper:Installing mitigation object onto node #" <<
                node->GetId ());

  // create jammer object
  Ptr<JammingMitigation> mitigation = m_jammingMitigation.Create<JammingMitigation> ();
  if (mitigation == NULL)
    {
      NS_FATAL_ERROR ("Can not create requested mitigation object:" <<
                      m_jammingMitigation.GetTypeId ().GetName ());
      return NULL;
    }

  // set jamming mitigation ID
  mitigation->SetId (node->GetId ());

  // check if utility already exists
  Ptr<WirelessModuleUtility> util = node->GetObject<WirelessModuleUtility> ();
  if (util == 0)
    {
      NS_LOG_DEBUG ("JammingMitigationHelper:WirelessModuleUtility doesn't exist, creating!");
      // utility doesn't exist, install one.
      WirelessModuleUtilityHelper utilHelper;
      utilHelper.Install (node);
      // get pointer to utility
      util = node->GetObject<WirelessModuleUtility> ();
    }
  else
    {
      NS_LOG_DEBUG ("JammingMitigationHelper:WirelessModuleUtility already installed!");
    }
  // set pointer to utility
  mitigation->SetUtility (util);
  // set callbacks
 
  util->SetEndRxCallback (MakeCallback (&ns3::JammingMitigation::EndRxHandler, mitigation));
  util->SetStartRxCallback (MakeCallback (&ns3::JammingMitigation::StartRxHandler, mitigation));
  util->SetEndTxCallback (MakeCallback (&ns3::JammingMitigation::EndTxHandler, mitigation));

  // check & set energy model
  Ptr<EnergySourceContainer> sourceContainer = node->GetObject<EnergySourceContainer> ();
  if (sourceContainer == NULL)
    {
      NS_FATAL_ERROR ("JammingMitigationHelper:Energy source doesn't exist!");
    }
  else
    {
      /*
       * XXX Force single energy source when using jammers.
       */
      NS_ASSERT (sourceContainer->GetN () == 1);
      mitigation->SetEnergySource (sourceContainer->Get (0));
    }

  // aggregate jammer to node
  node->AggregateObject (mitigation);

  return mitigation;
}

} // namespace ns3
