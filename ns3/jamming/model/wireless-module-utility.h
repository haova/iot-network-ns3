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

#ifndef WIRELESS_MODULE_UTILITY_H
#define WIRELESS_MODULE_UTILITY_H

#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-value.h"
#include "ns3/node.h"

namespace ns3 {

/**
 * \brief Utilities module for wireless nodes.
 *
 * This class implements a utility modules which will provide basic PHY layer
 * functionalities and information to intelligence layer classes (jammer and/or
 * jamming mitigation). Currently this class is tailored to work with the wifi
 * protocol.
 */
class WirelessModuleUtility : public Object
{
public:
  /**
   * Structure to keep the basic information about the underlying PHY layer.
   * Only include parameters that are not protocol-specific.
   */
  typedef struct PhyLayerInfo
  {
    double minTxPowerW;   // minimum TX power for PHY layer in Watts
    double maxTxPowerW;   // minimum TX power for PHY layer in Watts
    double TxGainDb;      // Tx gain, in dB
    double RxGainDb;      // Rx gain, in dB
    uint32_t phyRate;     // PHY rate in bits per second
    uint16_t numOfChannels;   // number of channels
    uint16_t currentChannel;  // current channel number
    Time channelSwitchDelay;  // channel switch delay
  } PhyLayerInfo;


  /**
   * Different modes for PHY layer send callback.
   */
  enum PacketSendMode
  {
    SEND_AS_JAMMER = 0,
    SEND_AS_HONEST,
    SEND_AS_OTHERS
  };

  /**
   * Callback type for RSS calculation in PHY.
   */
  typedef Callback<double> UtilityRssCallback;

  /**
   * Callback type for sending packet in PHY.
   */
  typedef Callback<void, Ptr<Packet>, double&, int> UtilitySendPacketCallback;

  /**
   * Callback for channel switch in PHY.
   */
  typedef Callback<void, uint16_t> UtilityChannelSwitchCallback;

  /**
   * Callback for start/end of TX.
   */
  typedef Callback<void, Ptr<Packet>, double> UtilityTxCallback;

  /**
   * Callback for handling start/end of RX.
   */
  typedef Callback<bool, Ptr<Packet>, double> UtilityRxCallback;

public:
  static TypeId GetTypeId (void);
  WirelessModuleUtility ();
  virtual ~WirelessModuleUtility ();

  // setters and getters for attributes
  void SetThroughputUpdateInterval (Time updateInterval);
  Time GetThroughputUpdateInterval (void) const;
  void SetPdrWindowSize (uint32_t pdrWindowSize);
  uint32_t GetPdrWindowSize (void) const;
  void SetRssUpdateInterval (Time updateInterval);
  Time GetRssUpdateInterval (void) const;

  // access functions to member variables
  void SetRssMeasurementCallback (UtilityRssCallback RssCallback);
  void SetInclusionList (std::vector<std::string> list);
  void SetExclusionList (std::vector<std::string> list);
  void SetSendPacketCallback (UtilitySendPacketCallback sendPacketCallback);
  void SetChannelSwitchCallback (UtilityChannelSwitchCallback channelSwitchCallback);
  void SetStartTxCallback (UtilityTxCallback startTxCallback);
  void SetEndTxCallback (UtilityTxCallback endTxCallback);
  void SetStartRxCallback (UtilityRxCallback startRxCallback);
  void SetEndRxCallback (UtilityRxCallback endRxCallback);
  void SetPhyLayerInfo (PhyLayerInfo info);
  PhyLayerInfo GetPhyLayerInfo (void) const;
  
  /**
   * \brief Handle a start of packet reception event.
   *
   * \param packet Pointer to the packet to be received.
   * \param startRss RSS at the start of the packet.
   * \returns True if the incoming packet is to be received. False if the packet
   * is to be dropped.
   *
   * Implementation of this function is based on YansWifiPhy class, it may not
   * be directly applicable to other wireless protocols. For reactive jammers,
   * it stops receiving the rest of the packet as soon as the preambles are
   * received. This function invokes the StartRxHandler callback in reactive
   * jammer to stop receiving current packet and start sending jamming signals.
   */
  bool StartRxHandler (Ptr<Packet> packet, double startRss);
 
  /**
   * \brief Handle an end of packet reception notification.
   *
   * \param packet Pointer to packet that has been received.
   * \param averageRss Average RSS over the reception of the packet.
   * \param isSuccessfullyReceived True if packets was received successfully.
   *
   * This function forwards the received packet to appropriate callbacks. It
   * is called inside by PHY layer driver at EndRx.
   */
  void EndRxHandler (Ptr<Packet> packet, double averageRss,
                     const bool isSuccessfullyReceived);
 					
  /**
   * \brief Handle the start of packet TX event.
   *
   * \param packet Pointer to the packet that is being sent.
   * \param txPower TX power used for the transmission of the packet.
   *
   * This function is called by PHY layer driver at the beginning of TX, used
   * for throughput recording.
   */
  void StartTxHandler (Ptr<const Packet> packet, double txPower);
 
  /**
   * \brief Handle the end of packet TX event.
   *
   * \param packet Pointer to the packet that is being sent.
   * \param txPower TX power used for the transmission of the packet.
   *
   * This function invokes appropriate callbacks for the end of TX. Mainly used
   * by jammers for scheduling jamming events.
   */
  void EndTxHandler (Ptr<const Packet> packet, double txPower);

  /**
   * \returns Total bytes received.
   */
  uint64_t GetTotalBytesRx (void) const;

  /**
   * \returns Total bytes sent.
   */
  uint64_t GetTotalBytesTx (void) const;

  /**
   * \returns Rx throughput.
   */
  double GetRxThroughput (void) const;

  /**
   * \returns Tx throughput.
   */
  double GetTxThroughput (void) const;

  /**
   * \returns Current PDR.
   */
  double GetPdr (void) const;

  /**
   * \returns Current node RSS.
   */
  double GetRss (void) const;

  /**
   * \brief Used by jammers to send out a jamming signal of a given power level
   * and duration.
   *
   * \param power double indicating the power of the jamming signal to be sent
   * \param duration Duration of the jamming burst
   * \returns TX power (in Watts) the signal is actually sent with.
   *
   * This function sends jamming signal of specific power via the PHY layer. If
   * the sending power is not supported by the PHY layer. The signal power will
   * be scaled to the nearest supported value.
   */
  double SendJammingSignal (double power, Time duration);

  /**
   * \brief This function switches the PHY channel number.
   *
   * \param channelNumber Desired channel number to switch to.
   */
  void SwitchChannel (uint16_t channelNumber);

  /**
   * \brief Sends jamming mitigation message (packet) at specified power.
   *
   * \param packet Mitigation message (packet) to be sent.
   * \param power Power (in Watts) to send the packet.
   * \returns Actual power the packet is set with.
   *
   * Similar to SendJammingSignal, this function will scale the signal power if
   * it is not supported by underlying PHY layer.
   */
  double SendMitigationMessage (Ptr<Packet> packet, double power);

   uint32_t GetTotalPkts ();
  uint32_t GetValidPkts();

private:
  void DoStart (void);
  void DoDispose (void);

  /**
   * \brief "Packet Monitor" that analyzes and records incoming packets.
   *
   * \param packet The incoming packet.
   * \param isSuccessfullyReceived True if packet was received successfully.
   *
   * This function is called by the EndRxHandler. It analyzes incoming packets
   * to extract and record packet type, length, etc. Filtering to analyze only
   * packets with certain headers and/or trailers can be done by specifying the
   * header/trailer inclusion and exclusion list.
   */
  void AnalyzeAndRecordIncomingPacket (const Ptr<Packet> packet,
                                       bool isSuccessfullyReceived);
  /**
   * \brief "Packet Monitor" that analyzes and records outgoing packets.
   *
   * This function can be called by StartTxHandler. See
   * AnalyzeAndRecordIncomingPacket().
   */
  void AnalyzeAndRecordOutgoingPacket (Ptr<const Packet> packet);

  /**
   * This function updates RX/TX throughput.
   */
  void UpdateThroughput (void);

  /**
   * \param packet Target packet.
   * \param headerName Name of header to search for
   * \returns Size of the header, 0 if header not found
   *
   * This function searches the incoming packet for desired header/trailer.
   * Modified from "void Packet::Print (std::ostream &os) const".
   */
  uint32_t FindHeader (Ptr<const Packet> packet, std::string headerName) const;

  /**
   * \brief Checks if headers/trailers in the inclusion list is in the given
   * packet.
   * \param packet Packet to check
   * \returns True if headers/trailers in the inclusion list are found in the
   * packet.
   *
   * This function checks if headers/trailers in the inclusion list are present
   * in the given packet. Used in UpdateThroughput function to exclude certain
   * types of packets. Note that if the inclusion list is empty (means we are
   * accepting packets with any header/trailer), this function returns true.
   */
  bool FindHeaderInInclusionList (Ptr<const Packet> packet);

  /**
   * \see FindHeaderInInclusionList ();
   */
  bool FindHeaderInExclusionList (Ptr<const Packet> packet);

  /**
   * \param isPacketValid True if packet is successfully received.
   *
   * This function updates PDR.
   */
  void UpdatePdr (const bool isPacketValid);

  /**
   * \param newPacketStatus True if packet is successfully received.
   *
   * This function inserts the packet status into the packet status list.
   */
  void InsertIntoPdrArray (bool newPacketStatus);

  /**
   * This function updates current RSS reading. Initially scheduled at
   * StartRxHandler ().
   */
  void UpdateRss (void);

  /**
   * \brief Convert dBm to Watts.
   *
   * \param dBm Original value.
   * \returns Converted value.
   */
  double DbmToW (double dBm) const;

  /**
   * \brief Convert Watts to dBm.
   *
   * \param w Original value.
   * \returns Converted value.
   */
  double WToDbm (double w) const;

 

private:
  PhyLayerInfo m_phyLayerInfo;
  
  // throughput recording
  TracedValue<uint64_t> m_totalBytesRx;  // variable keeping track of total bytes received
  TracedValue<uint64_t> m_totalBytesTx;  // variable keeping track of total bytes sent
  uint64_t m_previousTotalBytesRx;
  uint64_t m_previousTotalBytesTx;
  TracedValue<double> m_throughputRx; // RX throughput = derivative of total bytes RX
  TracedValue<double> m_throughputTx; // TX throughput = derivative of total bytes TX
  Time m_throughputUpdateInterval;    // throughput update interval
  EventId m_throughputUpdateEvent;    // event ID for throughput update event
  std::vector<std::string> m_headerInclusionList; // header/trailer inclusion list
  std::vector<std::string> m_headerExclusionList; // header/trailer exclusion list

  // PDR recording
  TracedValue<double> m_Pdr;    // variable keeping track of PDR
  uint32_t m_pdrWindowSize;     // PDR recording window size
  uint32_t m_numOfPktsRecvd;    // total # of packets received
  uint32_t m_pdrArrayCurIndex;  // current index of PDR array
  std::vector<bool> m_pktStatusRecord;  // List of validity of last N packets

  // RSS recording
  TracedValue<double> m_nodeRssW;   // current RSS reading at node, in watts, -1 indicates error
  Time m_rssUpdateInterval;         // RSS update interval
  EventId m_updateRssEvent;         // event ID for RSS update event
  TracedValue<double> m_avgPktRssW; // average packet RSS for previously received packet

  /**
   * Callback for measure current RSS (in watts). Done by PHY layer driver.
   * Returns -1 if no valid RSS calculation is available yet.
   *
   * The PHY layer driver for wifi is required to keep a copy of the latest
   * WifiMode it's currently operating on, for the interference helper to
   * calculate RSS. Implementation of callbacks is specific to the PHY layer
   * protocol. However the callback's format is generic and applicable for
   * different wireless protocols (PHY classes).
   */
  UtilityRssCallback m_rssMeasurementCallback;
  /**
   * Callback for sending a packet out through the PHY layer. 
   * 
   * Implementation of callbacks is specific to the PHY layer protocol. However
   * the callback's format is generic and applicable for different wireless
   * protocols (PHY classes).
   */
  UtilitySendPacketCallback m_sendPacketCallback;
  /**
   * Callback for switching channels in PHY.
   *
   * Implementation of callbacks is specific to the PHY. However the callback's
   * format is generic and applicable for different wireless protocols (PHY
   * classes). Note that this callback should not be set if channel switch is
   * not supported by the PHY.
   */
  UtilityChannelSwitchCallback m_channelSwitchCallback;
  /**
   * Callback to handle start of TX, not used for now.
   */
  UtilityTxCallback m_startTxCallback;
  /**
   * Callback to handle end of TX, used by jamming mitigation.
   */
  UtilityTxCallback m_endTxCallback;
  /**
   * Callback to handle start of RX, used by jammers.
   */
  UtilityRxCallback m_startRxCallback;
  /**
   * Callback for packet handler in jammer or jamming mitigation.
   */
  UtilityRxCallback m_endRxCallback;

  uint32_t  m_totalPkts;
  uint32_t m_validPkts;

};

} // namespace ns3

#endif /* WIRELESS_MODULE_UTILITY_H */
