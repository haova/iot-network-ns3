#ifndef NSL_WIFI_PHY_H
#define NSL_WIFI_PHY_H

#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-phy-standard.h"
#include "ns3/interference-helper.h"
#include "wireless-module-utility.h"



namespace ns3 {

class NslWifiChannel;
class WifiPhyStateHelper;
class WifiPpdu;


/**
 * \brief 802.11 PHY layer model
 * \ingroup wifi
 *
 * This PHY implements a model of 802.11a. The model
 * implemented here is based on the model described
 * in "Yet Another Network Simulator",
 * (http://cutebugs.net/files/wns2-yans.pdf).
 *
 *
 * This PHY model depends on a channel loss and delay
 * model as provided by the ns3::PropagationLossModel
 * and ns3::PropagationDelayModel classes, both of which are
 * members of the ns3::YansWifiChannel class.
 */
class NslWifiPhy : public WifiPhy
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  NslWifiPhy ();
  virtual ~NslWifiPhy ();

  /**
   * Set the YansWifiChannel this YansWifiPhy is to be connected to.
   *
   * \param channel the YansWifiChannel this YansWifiPhy is to be connected to
   */
  void SetChannel (const Ptr<NslWifiChannel> channel);

  /**
   * \param packet the packet to send
   * \param txVector the TXVECTOR that has tx parameters such as mode, the transmission mode to use to send
   *        this packet, and txPowerLevel, a power level to use to send this packet. The real transmission
   *        power is calculated as txPowerMin + txPowerLevel * (txPowerMax - txPowerMin) / nTxLevels
   * \param txDuration duration of the transmission.
   */
  void StartTx (Ptr<Packet> packet, WifiTxVector txVector, Time txDuration);

  virtual Ptr<Channel> GetChannel (void) const;

 void SetNode (Ptr<Node> node);
  void SetChannelNumber (uint16_t id);
  double MeasureRss (void);
  double WToDbm (double w) const;
  double RatioToDb (double ratio) const;
  double GetPowerDbm (uint8_t power) const;

  void SetRandomVariable (Ptr<RandomVariableStream>);
 
   int64_t AssignStreams (int64_t stream);
 

  void UtilitySendPacket (Ptr<Packet> packet, double &powerW, int utilitySendMode);
  void SendPacket (Ptr<const Packet> packet, WifiTxVector txMode, WifiPreamble preamble, uint8_t txPowerLevel);
  //double GetEdThresholdW (void) const;
  double DbmToW (double dbm) const;
  double DbToRatio (double db) const;
  void DriverStartTx (Ptr<const Packet> packet, double txPower);
  double GetRxNoiseFigure (void) const;
  
  void StartReceivePacket (Ptr<Packet> packet,
                           double rxPowerDbm,
                           WifiTxVector mode,
                           uint16_t preamble);
  


  void InitDriver (void);
  void DriverEndTx (Ptr<const Packet> packet, double txPower);
   void SetCurrentWifiMode (WifiMode mode);
    bool DriverStartRx (Ptr<Packet> packet, double startRssW);
    void EndReceive (Ptr<Packet> packet, Ptr<Event> event);
  double GetChannelFrequencyMhz() const;
   void DriverEndRx (Ptr<Packet> packet, double averageRssW,
                    const bool isSuccessfullyReceived);
  uint16_t GetChannelNumber () const;

  Ptr<RandomVariableStream> m_rand; 
 
protected:
  // Inherited
  virtual void DoDispose (void);
  void UpdatePhyLayerInfo (void);



private:
  Ptr<NslWifiChannel> m_channel; //!< YansWifiChannel that this YansWifiPhy is connected to
  Ptr<Node> m_node;  
  Ptr<WirelessModuleUtility> m_utility;
  bool m_isDriverInitialized;
  void DoStart (void);
  virtual void DoInitialize (void);
  WifiMode m_currentWifiMode;  
  WirelessModuleUtility::PhyLayerInfo m_phyLayerInfo; 
   WifiTxVector m_txVector;
   uint16_t m_channelNumber;
  Time m_channelSwitchDelay;
  
};

} //namespace ns3

#endif /*NSL_WIFI_PHY_H */
