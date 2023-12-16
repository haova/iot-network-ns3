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

#ifndef WIRELESS_MODULE_UTILITY_CONTAINER_H
#define WIRELESS_MODULE_UTILITY_CONTAINER_H

#include "ns3/wireless-module-utility.h"
#include <vector>
#include <stdint.h>

namespace ns3 {

/**
 * \brief Holds a vector of ns3::WirelessModuleUtility pointers.
 *
 * JammerHelper returns a list of WirelessModuleUtility pointers installed onto
 * nodes. Users can use this list to access WirelessModuleUtility objects on a
 * node easily.
 *
 * \see NetDeviceContainer
 */
class WirelessModuleUtilityContainer
{
public:
  typedef std::vector< Ptr<WirelessModuleUtility> >::const_iterator Iterator;

public:
  /**
   * Creates an empty WirelessModuleUtilityContainer.
   */
  WirelessModuleUtilityContainer ();

  /**
   * \param source Pointer to an WirelessModuleUtility.
   *
   * Creates an WirelessModuleUtilityContainer with exactly one WirelessModuleUtility
   * object previously instantiated.
   */
  WirelessModuleUtilityContainer (Ptr<WirelessModuleUtility> utility);

  /**
   * \param utilityName Name of WirelessModuleUtility.
   *
   * Creates an WirelessModuleUtilityContainer with exactly one WirelessModuleUtility
   * object previously instantiated and assigned a name using the Object name
   * service. This WirelessModuleUtility is specified by its assigned name.
   */
  WirelessModuleUtilityContainer (std::string utilityName);

  /**
   * \param a A WirelessModuleUtilityContainer.
   * \param b Another WirelessModuleUtilityContainer.
   *
   * Creates an WirelessModuleUtilityContainer by concatenating container b to a.
   *
   * \note Can be used to concatenate 2 Ptr<WirelessModuleUtility> directly. C++
   * will be calling WirelessModuleUtilityContainer constructor with
   * Ptr<WirelessModuleUtility> first.
   */
  WirelessModuleUtilityContainer (const WirelessModuleUtilityContainer &a,
                                  const WirelessModuleUtilityContainer &b);

  /**
   * \brief Get an iterator which refers to the first WirelessModuleUtility
   * pointer in the container.
   *
   * \returns An iterator which refers to the first WirelessModuleUtility in
   * container.
   *
   * WirelessModuleUtility Objects can be retrieved from the container in two
   * ways. First, directly by an index into the container, and second, using an
   * iterator. This method is used in the iterator method and is typically used
   * in a for-loop to run through the WirelessModuleUtility Objects.
   *
   * \code
   *   WirelessModuleUtilityContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some WirelessModuleUtility method
   *     }
   * \endcode
   */
  Iterator Begin (void) const;

  /**
   * \brief Get an iterator which refers to the last WirelessModuleUtility
   * pointer in the container.
   *
   * \returns An iterator which refers to the last WirelessModuleUtility in
   * container.
   *
   * WirelessModuleUtility Objects can be retrieved from the container in two
   * ways. First, directly by an index into the container, and second, using an
   * iterator. This method is used in the iterator method and is typically used
   * in a for-loop to run through the WirelessModuleUtility Objects.
   *
   * \code
   *   WirelessModuleUtilityContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some WirelessModuleUtility method
   *     }
   * \endcode
   */
  Iterator End (void) const;

  /**
   * \brief Get the number of Ptr<WirelessModuleUtility> stored in this
   * container.
   *
   * \returns The number of Ptr<WirelessModuleUtility> stored in this container.
   */
  uint32_t GetN (void) const;

  /**
   * \brief Get the i-th Ptr<WirelessModuleUtility> stored in this container.
   *
   * \param i Index of the requested Ptr<WirelessModuleUtility>.
   * \returns The requested Ptr<WirelessModuleUtility>.
   */
  Ptr<WirelessModuleUtility> Get (uint32_t i) const;

  /**
   * \param container Another WirelessModuleUtilityContainer to append.
   *
   * Appends the contents of another WirelessModuleUtilityContainer to the end
   * of this WirelessModuleUtilityContainer.
   */
  void Add (WirelessModuleUtilityContainer container);

  /**
   * \brief Append a single Ptr<WirelessModuleUtility> to the end of this
   * container.
   *
   * \param source Pointer to an WirelessModuleUtility.
   */
  void Add (Ptr<WirelessModuleUtility> utility);

  /**
   * \brief Append a single Ptr<WirelessModuleUtility> referred to by its object
   * name to the end of this container.
   *
   * \param utilityName Name of WirelessModuleUtility object.
   */
  void Add (std::string utilityName);


private:
  std::vector< Ptr<WirelessModuleUtility> > m_utilities;

};

} // namespace ns3

#endif /* WIRELESS_MODULE_UTILITY_CONTAINER_H */
