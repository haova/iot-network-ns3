/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009,2010 University of Washington
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

#ifndef WIRELESS_MODULE_UTILITY_HELPER
#define WIRELESS_MODULE_UTILITY_HELPER

#include "ns3/node.h"
#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/wireless-module-utility.h"
#include "wireless-module-utility-container.h"
#include <string>

namespace ns3 {

/**
 * \brief Create wireless utility objects \ref ns3::WirelessModuleUtility.
 */
class WirelessModuleUtilityHelper
{
public:
  WirelessModuleUtilityHelper ();
  ~WirelessModuleUtilityHelper ();

  /**
   * \param type The type of wireless module utility to use.
   *
   * Sets the type of wireless module utility to create for Install method.
   */
  void SetWirelessModuleUtilityType (std::string type);

  /**
   * \returns The type name of WirelessModuleUtility to create.
   */
  std::string GetWirelessModuleUtilityType (void) const;

  /**
   * \brief Sets one of the attributes of underlying WirelessModuleUtility.
   *
   * \param name Name of attribute to set.
   * \param v Value of the attribute.
   */
  void Set (std::string name, const AttributeValue &v);

  /**
   * \brief Sets the header/trailer inclusion list to a existing list.
   *
   * \param list Inclusion list.
   */
  void SetInclusionList (std::vector<std::string> list);

  /**
   * \brief Sets the header/trailer exclusion list to a existing list.
   *
   * \param list Exclusion list.
   */
  void SetExclusionList (std::vector<std::string> list);

  /**
   * \brief Sets the RSS measurement callback.
   *
   * \param RssCallback RSS measurement callback.
   */
  void SetRssMeasurementCallback (Callback<double> RssCallback);

  /**
   * \param node The node on which a utility object must be created.
   * \returns Container contains all the JammingMitigation object created by
   * this method.
   */
  WirelessModuleUtilityContainer Install (Ptr<Node> node) const;

  /**
   * \param c The set of nodes on which a utility object must be created.
   * \returns Container contains all the JammingMitigation object created by
   * this method.
   */
  WirelessModuleUtilityContainer Install (NodeContainer c) const;

  /**
   * \param nodeName The name of node on which a utility object must be created.
   * \returns Container contains all the JammingMitigation object created by
   * this method.
   */
  WirelessModuleUtilityContainer Install (std::string nodeName) const;

  /**
   * Install on *ALL* nodes exists in simulation.
   * \returns Container contains all the JammingMitigation object created by
   * this method.
   */
  WirelessModuleUtilityContainer InstallAll (void);


private:
  /**
   * \brief Installs a JammingMitigation object onto the specified node.
   *
   * \param node Pointer to node to install the JammingMitigation object.
   * \returns Pointer to the newly installed JammingMitigation object.
   */
  virtual Ptr<WirelessModuleUtility> DoInstall (Ptr<Node> node) const;

private:
  ObjectFactory m_wirelessUtility;

  Callback<double> m_rssMeasurementCallback; // RSS measurement callback

  std::vector<std::string> m_headerInclusionList; // header/trailer inclusion list
  std::vector<std::string> m_headerExclusionList; // header/trailer exclusion list

};

} // namespace ns3

#endif
