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

#ifndef JAMMING_MITIGATION_CONTAINER_H
#define JAMMING_MITIGATION_CONTAINER_H

#include "ns3/jamming-mitigation.h"
#include <vector>
#include <stdint.h>

namespace ns3 {

/**
 * \brief Holds a vector of ns3::JammingMitigation pointers.
 *
 * JammerHelper returns a list of JammingMitigation pointers installed onto
 * nodes. Users can use this list to access JammingMitigation objects on a node
 * easily.
 *
 * \see NetDeviceContainer
 */
class JammingMitigationContainer
{
public:
  typedef std::vector< Ptr<JammingMitigation> >::const_iterator Iterator;

public:
  /**
   * Creates an empty JammingMitigationContainer.
   */
  JammingMitigationContainer ();

  /**
   * \param source Pointer to an JammingMitigation.
   *
   * Creates an JammingMitigationContainer with exactly one JammingMitigation
   * object previously instantiated.
   */
  JammingMitigationContainer (Ptr<JammingMitigation> mitigation);

  /**
   * \param mitigationName Name of JammingMitigation.
   *
   * Creates an JammingMitigationContainer with exactly one JammingMitigation
   * object previously instantiated and assigned a name using the Object name
   * service. This JammingMitigation object is specified by its assigned name.
   */
  JammingMitigationContainer (std::string mitigationName);

  /**
   * \param a A JammingMitigationContainer.
   * \param b Another JammingMitigationContainer.
   *
   * Creates an JammingMitigationContainer by concatenating container b to a.
   *
   * \note Can be used to concatenate 2 Ptr<JammingMitigation> directly. C++
   * will be calling JammingMitigationContainer constructor with
   * Ptr<JammingMitigation> first.
   */
  JammingMitigationContainer (const JammingMitigationContainer &a,
                              const JammingMitigationContainer &b);

  /**
   * \brief Get an iterator which refers to the first JammingMitigation object
   * pointer in the container.
   *
   * \returns An iterator which refers to the first JammingMitigation object
   * pointer in container.
   *
   * JammingMitigation objects can be retrieved from the container in two ways.
   * First, directly by an index into the container, and second, using an
   * iterator. This method is used in the iterator method and is typically used
   * in a for-loop to run through the JammingMitigation objects.
   *
   * \code
   *   JammingMitigationContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some JammingMitigation method
   *     }
   * \endcode
   */
  Iterator Begin (void) const;

  /**
   * \brief Get an iterator which refers to the last JammingMitigation object
   * pointer in the container.
   *
   * \returns An iterator which refers to the last JammingMitigation object
   * pointer in container.
   *
   * JammingMitigation objects can be retrieved from the container in two ways.
   * First, directly by an index into the container, and second, using an
   * iterator. This method is used in the iterator method and is typically used
   * in a for-loop to run through the JammingMitigation objects.
   *
   * \code
   *   JammingMitigationContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some JammingMitigation method
   *     }
   * \endcode
   */
  Iterator End (void) const;

  /**
   * \brief Get the number of Ptr<JammingMitigation> stored in this container.
   *
   * \returns The number of Ptr<JammingMitigation> stored in this container.
   */
  uint32_t GetN (void) const;

  /**
   * \brief Get the i-th Ptr<JammingMitigation> stored in this container.
   *
   * \param i Index of the requested Ptr<JammingMitigation>.
   * \returns The requested Ptr<JammingMitigation>.
   */
  Ptr<JammingMitigation> Get (uint32_t i) const;

  /**
   * \param container Another JammingMitigationContainer to append.
   *
   * Appends the contents of another JammingMitigationContainer to the end of
   * this JammingMitigationContainer.
   */
  void Add (JammingMitigationContainer container);

  /**
   * \brief Append a single Ptr<JammingMitigation> to the end of this container.
   *
   * \param source Pointer to an JammingMitigation.
   */
  void Add (Ptr<JammingMitigation> mitigation);

  /**
   * \brief Append a single Ptr<JammingMitigation> referred to by its object
   * name to the end of this container.
   *
   * \param mitigationName Name of JammingMitigation object.
   */
  void Add (std::string mitigationName);


private:
  std::vector< Ptr<JammingMitigation> > m_mitigators;

};

} // namespace ns3

#endif /* JAMMING_MITIGATION_CONTAINER_H */
