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

#ifndef DETECTION_H
#define DETECTION_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/wireless-module-utility.h"
#include "ns3/energy-source.h"

namespace ns3 {

/**
 * Base class for jamming detection/mitigation methods. Other types of jamming
 * detection/mitigation methods can be defined from this class. Note that this
 * class requires a \ref WirelessModuleUtility object to operate, which is done
 * by \ref JammingMitigationHelper.
 */
class Detection : public Object
{
public:
  static TypeId GetTypeId (void);
  Detection ();
  virtual ~Detection ();

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
   * \brief Sets jamming mitigation object ID.
   *
   * \param id Jamming mitigation object ID.
   *
   * Jamming mitigation object ID is set to be the same as the node's ID on
   * which the jamming mitigation object is aggregated.
   */
  void SetId (uint32_t id);

  /**
   * \brief Gets jamming mitigation ID.
   *
   * \returns jamming mitigation ID.
   */
  uint32_t GetId (void) const;

  /**
   * Engages mitigation of jamming.
   */
  void StartDetection (void);

  /**
   * Stops mitigation of jamming.
   */
  void StopDetection (void);

  /**
   * \param packet Pointer to incoming packet.
   * \param startRss Start RSS of packet.
   * \returns True. Honest nodes will always accept the packet.
   *
   * This function is called by WirelessModuleUtility at start of RX. Signature
   * is WirelessModuleUtility::UtilityRxCallback .
   */
  bool StartRxHandler (Ptr<Packet> packet, double startRss);

  /**
   * \param packet Pointer to incoming packet.
   * \param Average RSS of packet.
   * \returns True. Honest nodes will always accept the packet.
   *
   * Handles incoming packet at end of RX from utility module, calls
   * DoHandleIncomingPacket.
   */
  bool EndRxHandler (Ptr<Packet> packet, double averageRss);

  /**
   * \brief Handles end TX event.
   *
   * \param packet Pointer to packet sent.
   * \param txPower Transmit power of packet.
   */
  void EndTxHandler (Ptr<Packet> packet, double txPower);

private:
  /**
   * \brief Engages mitigation of jamming.
   *
   * Implemented in child classes.
   */
  virtual void DoDetection (void) = 0;

  /**
   * \brief Stops mitigation of jamming.
   *
   * Implemented in child classes.
   */
  virtual void DoStopDetection (void) = 0;

  /**
   * \param packet Pointer to incoming packet.
   * \param startRss Start RSS of packet.
   *
   * Implemented in child classes.
   */
  virtual void DoStartRxHandler (Ptr<Packet> packet, double startRss) = 0;

  /**
   * \brief Handles incoming packet.
   *
   * \param packet Pointer to incoming packet.
   * \param Average RSS of packet.
   *
   * Implemented in child classes.
   */
  virtual void DoEndRxHandler (Ptr<Packet> packet, double averageRss) = 0;

  /**
   * \brief Handles end TX event.
   *
   * \param packet Pointer to packet sent.
   *
   * Implemented in child classes.
   */
  virtual void DoEndTxHandler (Ptr<Packet> packet, double txPower) = 0;

private:
  bool m_detectionOn;
  uint32_t m_id;


protected:
  bool IsDetectionOn (void) const;

};

} // namespace ns3

#endif /* DETECTION_H */
