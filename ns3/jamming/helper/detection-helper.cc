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

#include "detection-helper.h"
#include "ns3/energy-source-container.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/names.h"

NS_LOG_COMPONENT_DEFINE ("DetectionHelper");

namespace ns3 {

DetectionHelper::DetectionHelper ()
{
  m_detection.SetTypeId ("ns3::DetectionPer");
}

DetectionHelper::~DetectionHelper ()
{
}

void
DetectionHelper::SetDetectionType (std::string type)
{
  NS_LOG_FUNCTION (this);
  m_detection.SetTypeId (type);
}

std::string
DetectionHelper::GetDetectionType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_detection.GetTypeId ().GetName ();
}

void
DetectionHelper::Set (std::string name, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_detection.Set (name, v);
}

DetectionContainer
DetectionHelper::Install (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  return Install (NodeContainer (node));
}

DetectionContainer
DetectionHelper::Install (NodeContainer c) const
{
  NS_LOG_DEBUG (this);
  DetectionContainer container;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Detection> mitigator = DoInstall (*i);
      container.Add (mitigator);
    }
  return container;
}

DetectionContainer
DetectionHelper::Install (std::string nodeName) const
{
  NS_LOG_DEBUG (this);
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (node);
}

void
DetectionHelper::EnableLogComponents (void)
{
  LogComponentEnable ("Detection", LOG_LEVEL_ALL);
  LogComponentEnable ("DetectionPer", LOG_LEVEL_ALL);
}

/*
 * Private function starts here.
 */

Ptr<Detection>
DetectionHelper::DoInstall (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  NS_LOG_DEBUG ("DetectionHelper:Installing mitigation object onto node #" <<
                node->GetId ());

  // create detection object
  Ptr<Detection> mitigation = m_detection.Create<Detection> ();
  if (mitigation == NULL)
    {
      NS_FATAL_ERROR ("Can not create requested mitigation object:" <<
                      m_detection.GetTypeId ().GetName ());
      return NULL;
    }

  // set jamming mitigation ID
  mitigation->SetId (node->GetId ());

  // check if utility already exists
  Ptr<WirelessModuleUtility> util = node->GetObject<WirelessModuleUtility> ();
  if (util == 0)
    {
      NS_LOG_DEBUG ("DetectionHelper:WirelessModuleUtility doesn't exist, creating!");
      // utility doesn't exist, install one.
      WirelessModuleUtilityHelper utilHelper;
      utilHelper.Install (node);
      // get pointer to utility
      util = node->GetObject<WirelessModuleUtility> ();
    }
  else
    {
      NS_LOG_DEBUG ("DetectionHelper:WirelessModuleUtility already installed!");
    }
  // set pointer to utility
  mitigation->SetUtility (util);
  // set callbacks
  util->SetStartRxCallback (MakeCallback (&ns3::Detection::StartRxHandler, mitigation));
  util->SetEndRxCallback (MakeCallback (&ns3::Detection::EndRxHandler, mitigation));
  util->SetEndTxCallback (MakeCallback (&ns3::Detection::EndTxHandler, mitigation));

  // check & set energy model
  Ptr<EnergySourceContainer> sourceContainer = node->GetObject<EnergySourceContainer> ();
  if (sourceContainer == NULL)
    {
      NS_FATAL_ERROR ("DetectionnHelper:Energy source doesn't exist!");
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
