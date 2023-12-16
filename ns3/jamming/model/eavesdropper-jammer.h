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

#ifndef EAVESDROPPER_JAMMER_H
#define EAVESDROPPER_JAMMER_H

#include "jammer.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

namespace ns3 {

/**
 * Eavesdropper Jammer.
 */
class EavesdropperJammer : public Jammer
{
public:
  static TypeId GetTypeId (void);
  EavesdropperJammer ();
  virtual ~EavesdropperJammer ();

  /**
   * \brief Sets pointer to WirelessModuleUtility installed on node..
   *
   * \param utility Pointer to WirelessModuleUtility.
   */
  virtual void SetUtility (Ptr<WirelessModuleUtility> utility);

  /**
   * \brief Sets pointer to energy source.
   *
   * \param energySrcPtr Pointer to EnergySource installed on node.
   *
   * This function is called by JammerHelper.
   */
  virtual void SetEnergySource (Ptr<EnergySource> source);

  // setter & getters of attributes
  void SetRxTimeout (Time timeout);
  Time GetRxTimeout (void) const;
  void SetScanMode (bool mode);
  bool GetScanMode (void) const;
  void SetScanCycles (uint32_t cycles);
  uint32_t GetScanCycles (void) const;

  /**
   * Resets the packet count list for Eavesdropper scan mode.
   */
  void ClearPacketCountList (void);

private:
  void DoDispose (void);

  /**
   * Stops jammer.
   */
  void DoStopJammer (void);

  /**
   * Sends out jamming burst at random interval.
   */
  void DoJamming (void);

  /**
   * \brief Handles start RX event.
   *
   * \param packet Pointer to incoming packet.
   * \param startRss Start RSS of packet.
   * \returns True. Eavesdropper jammer will *always* accept incoming packets.
   */
  virtual bool DoStartRxHandler (Ptr<Packet> packet, double startRss);
  
  /**
   * \brief Handles end RX event (incoming packet).
   *
   * \param packet Pointer to incoming packet.
   *
   * Eavesdropper jammer will record the packet and traffic condition in the
   * current channel.
   */
  virtual bool DoEndRxHandler (Ptr<Packet> packet, double averageRss);

  /**
   * \brief Notifies jammer of end of sending jamming signal
   *
   * \param packet Pointer to dummy packet that was sent
   */
  virtual void DoEndTxHandler (Ptr<Packet> packet, double txPower);

  /**
   * This function is fired periodically. The period is defined to be how long
   * we want to scan each channel. When the function is fired, we will hop to
   * the next channel and start recording traffic condition.
   */
  void RxTimeoutHandler (void);

private:
  Ptr<WirelessModuleUtility> m_utility;
  Ptr<EnergySource> m_source;
  Time m_rxTimeout;           // interval between timeout events
  uint8_t m_numOfChannels;    // number of available channels
  EventId m_rxTimeoutEvent;   // rx timeout event
  uint32_t m_totalScanCycles; // # of cycles to scan through all channels
  uint32_t m_scanCount;       // # of channels scanned
  bool m_scanComplete;        // flag set to indicate scanning is complete
  bool m_isScan;              // set to run in scan mode
  /**
   * This list keeps a record of number of packets received in each channel.
   * Total number of channels depends on the PHY layer. The list is updated by
   * the eavesdropper jammer in scanning mode. It is used to determine which
   * channel has the most traffic.
   */
  std::vector<int> m_numOfPktsReceived;

};

} // namespace ns3

#endif  /* EAVESDROPPER_JAMMER_H */
