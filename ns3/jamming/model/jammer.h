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
 * Author: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

#ifndef JAMMER_H
#define JAMMER_H

#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/wireless-module-utility.h"
#include "ns3/energy-source.h"

namespace ns3 {

/**
 * Base class for wireless Jammers. Implements basic methods of jammers. Other
 * types of jammers can be defined from this class. Note that for the jammer to
 * work properly, a \ref WirelessModuleUtility must be installed onto the node.
 * This is automatically done by the \ref JammerHelper.
 */
class Jammer : public Object
{
public:
  static TypeId GetTypeId (void);
  Jammer ();
  virtual ~Jammer ();

  /**
   * \brief Sets pointer to WirelessModuleUtility installed on node..
   *
   * \param utility Pointer to WirelessModuleUtility.
   */
  virtual void SetUtility (Ptr<WirelessModuleUtility> utility) = 0;

  /**
   * \brief Sets pointer to energy source.
   *
   * \param energySrcPtr Pointer to EnergySource installed on node.
   *
   * This function is called by JammerHelper.
   */
  virtual void SetEnergySource (Ptr<EnergySource> source) = 0;

  /**
   * \brief Sets jammer ID.
   *
   * \param id Jammer ID.
   *
   * Jammer ID is set to be the same as the node's ID on which the jammer is
   * aggregated.
   */
  void SetId (uint32_t id);

  /**
   * \brief Gets jammer ID.
   *
   * \returns Jammer ID.
   */
  uint32_t GetId (void) const;

  /**
   * Starts the jammer.
   */
  void StartJammer (void);

  /**
   * Stops jammer, calls DoStopJammer.
   */
  void StopJammer (void);

  /**
   * \param packet Pointer to incoming packet.
   * \param startRss Start RSS of packet.
   * \returns True if the incoming packet is to be received; false if the packet
   * is to be dropped.
   *
   * This function is called by WirelessModuleUtility at start of RX. Signature
   * is WirelessModuleUtility::UtilityRxCallback .
   */
  bool StartRxHandler (Ptr<Packet> packet, double startRss);
  
  /**
   * \param packet Pointer to incoming packet.
   * \param averageRss Average RSS of packet.
   * \returns True if packet is received. False otherwise.
   *
   * This function is called by WirelessModuleUtility at end of RX. Signature
   * is WirelessModuleUtility::UtilityRxCallback .
   */
  bool EndRxHandler (Ptr<Packet> packet, double averageRss);

  /**
   * \param packet Pointer to sent packet.
   *
   * This function is called by WirelessModuleUtility at end of TX. Signature
   * is WirelessModuleUtility::UtilityTxCallback .
   */
  void EndTxHandler (Ptr<Packet> packet, double txPower);

  /**
   * \returns True if jammer is on.
   */
  bool IsJammerOn (void) const;


private:
  /**
   * \brief Performs jamming.
   *
   * Implemented in child classes.
   */
  virtual void DoJamming (void) = 0;

  /**
   * \brief Stops jammer.
   *
   * Implemented in child classes.
   */
  virtual void DoStopJammer (void) = 0;

  /**
   * \brief Handles start RX event.
   *
   * \param packet Pointer to incoming packet.
   * \param startRss RSS at the start of packet.
   * \returns True if this packet is to be received, false otherwise.
   *
   * Implemented in child classes.
   */
  virtual bool DoStartRxHandler (Ptr<Packet> packet, double startRss) = 0;

  /**
   * \brief Handles incoming packet (EndRxHandler).
   *
   * \param packet Pointer to incoming packet.
   * \param averageRss Average RSS of packet.
   * \returns True.
   *
   * Implemented in child classes.
   */
  virtual bool DoEndRxHandler (Ptr<Packet> packet, double averageRss) = 0;

  /**
   * \brief Handles end TX event.
   *
   * \param packet Pointer to outgoing packet.
   *
   * Implemented in child classes
   */
  virtual void DoEndTxHandler (Ptr<Packet> packet, double txPower) = 0;

private:
  uint32_t m_id;
  bool m_jammerOn;

};

} // namespace ns3

#endif /* JAMMER_H */
