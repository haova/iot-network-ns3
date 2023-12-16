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

#ifndef JAMMER_CONTAINER_H
#define JAMMER_CONTAINER_H

#include "ns3/jammer.h"
#include <vector>
#include <stdint.h>

namespace ns3 {

/**
 * \brief Holds a vector of ns3::Jammer pointers.
 *
 * JammerHelper returns a list of Jammer pointers installed onto nodes. Users
 * can use this list to access Jammer objects on a node easily.
 *
 * \see NetDeviceContainer
 */
class JammerContainer
{
public:
  typedef std::vector< Ptr<Jammer> >::const_iterator Iterator;

public:
  /**
   * Creates an empty JammerContainer.
   */
  JammerContainer ();

  /**
   * \param source Pointer to an Jammer.
   *
   * Creates an JammerContainer with exactly one Jammer previously instantiated.
   */
  JammerContainer (Ptr<Jammer> jammer);

  /**
   * \param jammerName Name of Jammer.
   *
   * Creates an JammerContainer with exactly one Jammer previously instantiated
   * and assigned a name using the Object name service. This Jammer is specified
   * by its assigned name.
   */
  JammerContainer (std::string jammerName);

  /**
   * \param a A JammerContainer.
   * \param b Another JammerContainer.
   *
   * Creates an JammerContainer by concatenating JammerContainer b to
   * JammerContainer a.
   *
   * \note Can be used to concatenate 2 Ptr<Jammer> directly. C++ will be
   * calling JammerContainer constructor with Ptr<Jammer> first.
   */
  JammerContainer (const JammerContainer &a, const JammerContainer &b);

  /**
   * \brief Get an iterator which refers to the first Jammer pointer in
   * the container.
   *
   * \returns An iterator which refers to the first Jammer in container.
   *
   * Jammer can be retrieved from the container in two ways. First, directly by
   * an index into the container, and second, using an iterator. This method is
   * used in the iterator method and is typically used in a for-loop to run
   * through the Jammer.
   *
   * \code
   *   JammerContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some Jammer method
   *     }
   * \endcode
   */
  Iterator Begin (void) const;

  /**
   * \brief Get an iterator which refers to the last Jammer pointer in the
   * container.
   *
   * \returns An iterator which refers to the last Jammer in container.
   *
   * Jammer can be retrieved from the container in two ways. First, directly by
   * an index into the container, and second, using an iterator. This method is
   * used in the iterator method and is typically used in a for-loop to run
   * through the Jammer.
   *
   * \code
   *   JammerContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some Jammer method
   *     }
   * \endcode
   */
  Iterator End (void) const;

  /**
   * \brief Get the number of Ptr<Jammer> stored in this container.
   *
   * \returns The number of Ptr<Jammer> stored in this container.
   */
  uint32_t GetN (void) const;

  /**
   * \brief Get the i-th Ptr<Jammer> stored in this container.
   *
   * \param i Index of the requested Ptr<Jammer>.
   * \returns The requested Ptr<Jammer>.
   */
  Ptr<Jammer> Get (uint32_t i) const;

  /**
   * \param container Another JammerContainer to append.
   *
   * Appends the contents of another JammerContainer to the end of this
   * JammerContainer.
   */
  void Add (JammerContainer container);

  /**
   * \brief Append a single Ptr<Jammer> to the end of this container.
   *
   * \param source Pointer to an Jammer.
   */
  void Add (Ptr<Jammer> jammer);

  /**
   * \brief Append a single Ptr<Jammer> referred to by its object name to
   * the end of this container.
   *
   * \param jammerName Name of Jammer object.
   */
  void Add (std::string jammerName);


private:
  std::vector< Ptr<Jammer> > m_jammers;

};

} // namespace ns3

#endif /* JAMMER_CONTAINER_H */
