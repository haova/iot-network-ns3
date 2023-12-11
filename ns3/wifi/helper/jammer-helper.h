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

#ifndef JAMMER_HELPER_H
#define JAMMER_HELPER_H

#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/ptr.h"
#include "ns3/wireless-module-utility.h"
#include "ns3/jammer.h"
#include "wireless-module-utility-helper.h"
#include "jammer-container.h"

namespace ns3 {

/**
 * \brief Creates Jammer objects \ref ns3::Jammer.
 *
 * Note that the jammer object depends on the WirelessModuleUtility object. The
 * helper will make sure a WirelessModuleUtility object is installed before
 * installing jammer.
 *
 * New types of jammers can reuse most of this base class by inheriting from
 * this base class and overwriting the DoInstall method.
 *
 * Note that this helper installs ns3::ConstantJammer by default.
 */
class JammerHelper
{
public:
  JammerHelper ();
  ~JammerHelper ();

  /**
   * \param type The type of jammer to create.
   *
   * Sets the type of jammer to create for Install method.
   */
  void SetJammerType (std::string type);

  /**
   * \returns The name of the type of jammer to create.
   */
  std::string GetJammerType (void) const;

  /**
   * \param name Name of attribute to set.
   * \param v Value of the attribute.
   *
   * Sets one of the attributes of underlying Jammer.
   */
  void Set (std::string name, const AttributeValue &v);

  /**
   * \param node The node on which a jammer object must be created.
   * \returns Container contains all the Jammer object created by this method.
   */
  JammerContainer Install (Ptr<Node> node) const;

  /**
   * \param c The set of nodes on which a jammer object must be created.
   * \returns Container contains all the Jammer object created by this method.
   */
  JammerContainer Install (NodeContainer c) const;

  /**
   * \param nodeName The name of node on which a jammer object must be created.
   * \returns Container contains all the Jammer object created by this method.
   */
  JammerContainer Install (std::string nodeName) const;

  /**
   * Helper to enable all Jammer log components with one statement.
   */
  static void EnableLogComponents (void);


private:
  /**
   * \brief Installs a Jammer onto the specified node.
   *
   * \param node Pointer to node to install the jammer.
   * \returns Pointer to the newly installed jammer.
   */
  virtual Ptr<Jammer> DoInstall (Ptr<Node> node) const;

private:
  ObjectFactory m_jammer;

};

} // namespace ns3

#endif /* JAMMER_HELPER_H */
