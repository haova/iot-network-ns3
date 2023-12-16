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

#ifndef DETECTION_PER_H
#define DETECTION_PER_H

#include "detection.h"
#include "ns3/nstime.h"
#include "ns3/rng-stream.h"

namespace ns3 {

/**
 * Detects and mitigates jamming by hopping onto a different wireless channel.
 *
 * There are 3 jamming detection techniques implemented in this class:
 *  1.  PDR (Packet Delivery Ratio) only. Declares node jammed if PDR is above
 *      certain threshold.
 *  2.  RSS (Received Signal Strength) only. Declares node jammed if RSS is
 *      above certain threshold.
 *  3.  PDR & RSS. Declares node jammed if both PDR and RSS are above certain
 *      thresholds. Note that PDR and RSS have equal weight in this method.
 *
 * Once jamming is detected, a channel hop message (specified by user) is sent
 * to all of the jammed node's neighbors. Then the sender will hop to a another
 * channel. Once a node receives a channel hop message, it will then hop to the
 * same channel as the sender of the channel hop message.
 *
 * The channel hopping sequence is the same for all honest nodes installed with
 * this class. The channel sequence is generated randomly on each nodes with a
 * pre-agreed (specified by user before simulation) seed. Therefore this random
 * sequence will be the same on all honest nodes.
 */
class DetectionPer : public Detection
{
public:
  /**
   * List of metrics to use for detecting presence of jamming.
   */
  enum JammingDetectionMethod {
    PDR_ONLY = 0,//!< PDR_ONLY
    RSS_ONLY,    //!< RSS_ONLY
    PDR_AND_RSS  //!< PDR_AND_RSS
  };

public:
  static TypeId GetTypeId (void);
  DetectionPer ();
  virtual ~DetectionPer ();

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

  // setter and getter for attributes threshold
  void SetJammingDetectionMethod (JammingDetectionMethod method);
  uint32_t GetJammingDetectionMethod (void) const;
  void SetJammingDetectionThreshold (double threshold);
  double GetJammingDetectionThreshold (void) const;
  void SetTxPower (double txPower);
  double GetTxPower (void) const;
  void SetChannelHopMessage (std::string message);
  std::string GetChannelHopMessage (void) const;
  void SetChannelHopDelay (Time delay);
  Time GetChannelHopDelay (void) const;
  /**
   * \param seed 32-bit integer.
   *
   * Sets the seed used for the random sequence generator. This function also
   * re-initializes the RNG. Note, assigning same value to all 6 slots.
   */
  void SetRngSeed (uint32_t seed);
  uint32_t GetRngSeed (void) const;
  void SetStartChannelNumber (uint16_t channelNumber);
  uint16_t GetStartChannelNumber (void) const;
  void SetEndChannelNumber (uint16_t channelNumber);
  uint16_t GetEndChannelNumber (void) const;

  /**
   * \brief Calculates the degree of jamming.
   *
   * \param method Jamming detection method to use (JammingDetectionMethod).
   * \returns Degree of jamming at current node (0 - 1). 0 means not jammed.
   */
  double DegreeOfJamming (int method);

  /**
   * \brief Indicates if jamming is detected.
   *
   * \param method Jamming detection method to use (JammingDetectionMethod).
   * \returns True if jamming is detected.
   */
  bool IsJammingDetected (int method);

  /**
   * Sends channel hop package at specified tx power, then schedules a channel
   * hop at current node.
   */
  void SendChannelHopMessage (void);

  /**
   * Hop to next channel specified by the channel sequence list.
   */
  void HopChannel (void);


private:
  void DoStart (void);
  void DoDispose (void);

  /**
   * This function starts the mitigation process. For mitigation by channel hop
   * this function is empty.
   */
  virtual void DoDetection (void);

  /**
   * This function stops the mitigation process. For mitigation by channel hop
   * this function is empty.
   */
  virtual void DoStopDetection (void);

  /**
   * \brief Handles start RX event.
   *
   * \param packet Pointer to incoming packet.
   * \param startRss Start RSS of packet.
   */
  virtual void DoStartRxHandler (Ptr<Packet> packet, double startRss);

  /**
   * \brief Handles incoming packet from utility.
   *
   * \param packet Pointer to incoming packet. NULL if packet reception failed.
   * \param Average RSS of packet.
   */
  virtual void DoEndRxHandler (Ptr<Packet> packet, double averageRss);

  /**
   * \brief Handles end TX event.
   *
   * \param packet Pointer to packet sent.
   */
  virtual void DoEndTxHandler (Ptr<Packet> packet, double txPower);

  /**
   * \brief Finds mitigation message in packet.
   *
   * \param packet Pointer to packet.
   * \param target Target string to look for.
   * \returns True if found.
   */
  bool FindMessage (Ptr<const Packet> packet, std::string target) const;

  /**
   * \returns Channel number for the node to hop to.
   *
   * This function is used to generate random sequence for channel hopping.
   */
  uint16_t RandomSequenceGenerator (void);
 

private:
  Ptr<WirelessModuleUtility> m_utility;
  Ptr<EnergySource> m_source;

  /**
   * Jamming detection method to use.
   */
  JammingDetectionMethod m_jammingDetectionMethod;
  /**
   * Threshold [0,1] >= which node is considered "jammed".
   */
  double m_jammingDetectionThreshold;
  /**
   * TX power for sending channel hop message.
   */
  double m_txPower;
  /**
   * \brief Channel hop message.
   *
   * This channel hop message is converted into bit stream and sent though the
   * channel. The message itself can be jammed. It is up to the user to decide
   * how long (what) the message is going to be and what power the message is
   * going to be sent with. Note that the longer the message is the more likely
   * that it will be jammed.
   */
  std::string m_channelHopMessage;
  /**
   * Delay before a node issues the channel hop command. If set to 0 (default)
   * the node will switch channel as soon as mitigation message is sent.
   */
  Time m_channelHopDelay;
  /**
   * RNG used to generate random stream.
   */
  RngStream *m_stream;
  /**
   * Pointer to seed of RNG.
   */
  uint32_t m_seed;
  /**
   * Starting channel number.
   */
  uint16_t m_channelStart;
  /**
   * Ending channel number
   */
  uint16_t m_channelEnd;
  /**
   * Flag to indicate whether the RNG has been initialized.
   */
  bool m_rngInitialized;
  /**
   * Flag set true if we are waiting to hop channel. Checked only at
   * EndTxHandler. Set at SendChannelHopMessage. Reset at HopChannel.
   */
  bool m_waitingToHop;
  /**
   * Start RSS of a packet.
   */
  double m_startRss;
  /**
   * Average RSS of a packet.
   */
  double m_averageRss;
};

} // namespace ns3

#endif  /* MITIGATE_BY_CHANNEL_HOP_H */
