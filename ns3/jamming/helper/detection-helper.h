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

#ifndef DETECTION_HELPER_H
#define DETECTION_HELPER_H

#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/ptr.h"
#include "ns3/wireless-module-utility.h"
#include "ns3/detection.h"
#include "wireless-module-utility-helper.h"
#include "detection-container.h"

namespace ns3 {

/**
 * \brief Creates JammingMitigation objects \ref ns3::JammingMitigation.
 *
 * Note that the JammingMitigation object depends on the WirelessModuleUtility
 * object. The helper will make sure a WirelessModuleUtility object is installed
 * before installing JammingMitigation object.
 *
 * New types of JammingMitigation object can reuse most of this base class by
 * inheriting from this base class and overwriting the DoInstall method.
 *
 * Note that this helper installs ns3::MitigateByChannelHop by default.
 */
class DetectionHelper
{
public:

  DetectionHelper ();
  ~DetectionHelper ();

  /**
   * \param type The type of mitigation object to create.
   *
   * Sets the type of mitigation object to create for Install method.
   */
  void SetDetectionType (std::string type);

  /**
   * \returns The type name of mitigation object to create for Install method.
   */
  std::string GetDetectionType (void) const;

  /**
   * \brief Sets one of the attributes of underlying mitigation object.
   *
   * \param name Name of attribute to set.
   * \param v Value of the attribute.
   */
  void Set (std::string name, const AttributeValue &v);

  /**
   * \param node The node on which a utility object must be created.
   * \returns Container contains all the JammingMitigation object created by
   * this method.
   */
  DetectionContainer Install (Ptr<Node> node) const;

  /**
   * \param c The set of nodes on which a utility object must be created.
   * \returns Container contains all the JammingMitigation object created by
   * this method.
   */
  DetectionContainer Install (NodeContainer c) const;

  /**
   * \param nodeName The name of node on which a utility object must be created.
   * \returns Container contains all the JammingMitigation object created by
   * this method.
   */
  DetectionContainer Install (std::string nodeName) const;

  /**
   * Helper to enable all JammingMitigation log components with one statement.
   */
  static void EnableLogComponents (void);


private:
  /**
   * \brief Installs a JammingMitigation object onto the specified node.
   *
   * \param node Pointer to node to install the JammingMitigation object.
   * \returns Pointer to the newly installed JammingMitigation object.
   */
  virtual Ptr<Detection> DoInstall (Ptr<Node> node) const;

private:
  ObjectFactory m_detection;

};

} // namespace ns3

#endif  /* DETECTION_HELPER_H */
