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

#include "jammer-helper.h"
#include "ns3/energy-source-container.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/names.h"

NS_LOG_COMPONENT_DEFINE ("JammerHelper");

namespace ns3 {

JammerHelper::JammerHelper ()
{
  m_jammer.SetTypeId ("ns3::ConstantJammer");
}

JammerHelper::~JammerHelper ()
{
}

void
JammerHelper::SetJammerType (std::string type)
{
  NS_LOG_FUNCTION (this);
  m_jammer.SetTypeId (type);
}

std::string
JammerHelper::GetJammerType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_jammer.GetTypeId ().GetName ();
}

void
JammerHelper::Set (std::string name, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_jammer.Set (name, v);
}

JammerContainer
JammerHelper::Install (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  return Install (NodeContainer (node));
}

JammerContainer
JammerHelper::Install (NodeContainer c) const
{
  NS_LOG_DEBUG (this);
  JammerContainer container;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Jammer> jammer = DoInstall (*i);
      container.Add (jammer);
    }
  return container;
}

JammerContainer
JammerHelper::Install (std::string nodeName) const
{
  NS_LOG_DEBUG (this);
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (node);
}

void
JammerHelper::EnableLogComponents (void)
{
  LogComponentEnable ("Jammer", LOG_LEVEL_ALL);
  LogComponentEnable ("ReactiveJammer", LOG_LEVEL_ALL);
  LogComponentEnable ("RandomJammer", LOG_LEVEL_ALL);
  LogComponentEnable ("ConstantJammer", LOG_LEVEL_ALL);
  LogComponentEnable ("EavesdropperJammer", LOG_LEVEL_ALL);
}

/*
 * Private functions start here.
 */
Ptr<Jammer>
JammerHelper::DoInstall (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  NS_LOG_DEBUG ("JammerHelper:Installing jammer onto node #" << node->GetId ());

  // create jammer object
  Ptr<Jammer> jammer = m_jammer.Create<Jammer> ();
  if (jammer == NULL)
    {
      NS_FATAL_ERROR ("Can not create requested jammer object:" <<
                      m_jammer.GetTypeId ().GetName ());
      return NULL;
    }

  // set jammer ID to be the same as node ID
  jammer->SetId (node->GetId ());

  // check if utility already exists
  Ptr<WirelessModuleUtility> util = node->GetObject<WirelessModuleUtility> ();
  if (util == NULL)
    {
      NS_LOG_DEBUG ("JammerHelper:WirelessModuleUtility doesn't exist, creating!");
      // utility doesn't exist, install one.
      WirelessModuleUtilityHelper utilHelper;
      utilHelper.Install (node);
      // get pointer to utility
      util = node->GetObject<WirelessModuleUtility> ();
    }
  else
    {
      NS_LOG_DEBUG ("JammerHelper:WirelessModuleUtility already installed!");
    }
  // set utility
  jammer->SetUtility (util);
  // set callbacks
  util->SetEndRxCallback (MakeCallback(&ns3::Jammer::EndRxHandler, jammer));
  util->SetStartRxCallback (MakeCallback (&ns3::Jammer::StartRxHandler, jammer));
  util->SetEndTxCallback (MakeCallback (&ns3::Jammer::EndTxHandler, jammer));

  // check & set energy source
  Ptr<EnergySourceContainer> sourceContainer = node->GetObject<EnergySourceContainer> ();
  if (sourceContainer == NULL)
    {
      NS_FATAL_ERROR ("JammerHelper:Energy source doesn't exist!");
    }
  else
    {
      /*
       * XXX Force single energy source when using jammers.
       */
      NS_ASSERT (sourceContainer->GetN () == 1);
      jammer->SetEnergySource (sourceContainer->Get (0));
    }

  // aggregate jammer to node
  node->AggregateObject (jammer);

  return jammer;
}

} // namespace ns3
