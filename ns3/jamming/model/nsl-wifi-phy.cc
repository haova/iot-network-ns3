#include "nsl-wifi-phy.h"
#include "wireless-module-utility.h"
#include "nsl-wifi-channel.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-phy-state-helper.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/error-rate-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/energy-source.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/pointer.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/wifi-preamble.h"
#include "ns3/error-model.h"
#include "ns3/string.h"

NS_LOG_COMPONENT_DEFINE ("NslWifiPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NslWifiPhy);


TypeId
NslWifiPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NslWifiPhy")
    .SetParent<WifiPhy> ()
    .SetGroupName ("Wifi")
    .AddConstructor<NslWifiPhy> ()
    .AddAttribute ("Rand", "The decision variable attached to this error model.",
                    StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                    MakePointerAccessor (&NslWifiPhy::m_rand),
                    MakePointerChecker<RandomVariableStream> ())
  ;
  return tid;
}

NslWifiPhy::NslWifiPhy ()
:m_isDriverInitialized (false),
m_channelNumber(1)
{
  NS_LOG_FUNCTION (this);
  m_txVector = WifiTxVector();
  m_channelSwitchDelay = MicroSeconds (250);
  
}

NslWifiPhy::~NslWifiPhy ()
{
  NS_LOG_FUNCTION (this);
  m_txVector = WifiTxVector();
}
void
NslWifiPhy::DoStart (void)
{
  NS_LOG_FUNCTION (this);
   // initialize driver at beginning of simulation
   InitDriver();
}

void
NslWifiPhy::DoInitialize(){
    NS_LOG_FUNCTION (this);
    InitDriver();
    //m_initialized=true;
}

void
NslWifiPhy::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
  m_node = 0;
  WifiPhy::DoDispose ();
 
}
uint16_t
NslWifiPhy::GetChannelNumber() const
{
  return m_channelNumber;
}

 void 
NslWifiPhy::SetRandomVariable (Ptr<RandomVariableStream> ranvar)
 {
   NS_LOG_FUNCTION (this << ranvar);
   m_rand = ranvar;
 }
 
 int64_t 
 NslWifiPhy::AssignStreams (int64_t stream)
 {
   NS_LOG_FUNCTION (this << stream);
   m_rand->SetStream (stream);
   return 1;
 }

Ptr<Channel>
NslWifiPhy::GetChannel (void) const
{
  return m_channel;
}

void
NslWifiPhy::SetChannel (const Ptr<NslWifiChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);
  m_channel = channel;
  m_channel->Add (this);
}

void
NslWifiPhy::StartTx (Ptr<Packet> packet, WifiTxVector txVector, Time txDuration)
{
  NS_LOG_DEBUG ("Start transmission: signal power before antenna gain=" << GetPowerDbm (txVector.GetTxPowerLevel ()) << "dBm");

  m_channel->Send (this, packet, GetPowerDbm (txVector.GetTxPowerLevel ()), txDuration,txVector.GetPreambleType (),txVector);
}

void
NslWifiPhy::SetNode (Ptr<Node> nodePtr)
{
  NS_LOG_FUNCTION (this << nodePtr);
  m_node = nodePtr;
}

double
NslWifiPhy::GetChannelFrequencyMhz() const
{
  return WifiPhy::GetFrequency () + 5 * (GetChannelNumber() - 1);
}


void
NslWifiPhy::SetCurrentWifiMode (WifiMode mode)
{
  NS_LOG_FUNCTION (this);
  m_currentWifiMode = mode;
  m_txVector.SetMode(mode);
  
  if (m_utility)
    {
      UpdatePhyLayerInfo ();
    }
  else
    {
      NS_LOG_DEBUG ("NslWifiPhy:Utility is *not* installed on Node #" << m_node->GetId ());
    }

}
double
NslWifiPhy::GetRxNoiseFigure (void) const
{
  return RatioToDb (m_interference.GetNoiseFigure ());
}

void
NslWifiPhy::InitDriver (void)
{
  NS_LOG_FUNCTION (this);
  Ptr<NetDevice> device = WifiPhy::GetDevice();
  m_node = device->GetNode ();
  SetCurrentWifiMode (WifiPhy::GetMode(0));
  
  if (!m_isDriverInitialized)
    {
      NS_LOG_DEBUG ("NslWifiPhy:Driver being initialized at Node #" << m_node->GetId ());
      // setting default wifi mode
      //SetCurrentWifiMode (m_modes[0]);
      m_utility = m_node->GetObject<WirelessModuleUtility> ();
      if (m_utility != NULL)
        {
          m_utility->SetRssMeasurementCallback (MakeCallback (&NslWifiPhy::MeasureRss, this));
          m_utility->SetSendPacketCallback (MakeCallback (&NslWifiPhy::UtilitySendPacket, this));
          m_utility->SetChannelSwitchCallback (MakeCallback (&NslWifiPhy::SetChannelNumber, this));
          UpdatePhyLayerInfo ();
        }
      m_isDriverInitialized = true;
    }
  else
    {
      NS_LOG_DEBUG ("NslWifiPhy:Driver already initialized at Node #" << m_node->GetId ());
    }
  // show some debug messages
  if (m_utility == NULL)
    {
      NS_LOG_DEBUG ("NslWifiPhy:Utility module is *not* installed on Node #" << m_node->GetId ());
    }
}

void
NslWifiPhy::UpdatePhyLayerInfo (void)
{
  NS_LOG_FUNCTION (this);
  m_phyLayerInfo.currentChannel = m_channelNumber;//WifiPhy::GetChannelNumber();
  m_phyLayerInfo.maxTxPowerW = DbmToW (WifiPhy::GetTxPowerEnd());
  m_phyLayerInfo.minTxPowerW = DbmToW (WifiPhy::GetTxPowerStart());
  m_phyLayerInfo.TxGainDb = WifiPhy::GetTxGain() ;
  m_phyLayerInfo.RxGainDb = WifiPhy::GetRxGain ();
  m_phyLayerInfo.numOfChannels = 11;  // XXX assuming US standard
  m_phyLayerInfo.phyRate = m_currentWifiMode.GetDataRate (WifiPhy::GetChannelWidth());
  m_phyLayerInfo.channelSwitchDelay =  WifiPhy::GetChannelSwitchDelay();
  // notify utility
  if (m_utility != NULL)
    {
      m_utility->SetPhyLayerInfo (m_phyLayerInfo);
    }
  else
    {
      NS_LOG_DEBUG ("NslWifiPhy:Utility is *not* installed on Node #" <<
                    m_node->GetId ());
    }
}


void
NslWifiPhy::UtilitySendPacket(Ptr<Packet> packet, double &powerW, int utilitySendMode)
{
  NS_LOG_FUNCTION (this << packet << powerW << utilitySendMode);

  // Convert power in Watts to a power level
  uint8_t powerLevel;
  powerLevel = (uint8_t)(WifiPhy::GetNTxPower() * (WToDbm(powerW) - WifiPhy::GetTxPowerStart()) /
               (WifiPhy::GetTxPowerEnd() - WifiPhy::GetTxPowerStart()));

  if (powerLevel >= 0 && powerLevel < WifiPhy::GetNTxPower())
    {
      NS_LOG_DEBUG ("NslWifiPhy:Inside send packet callback at node #" <<
                    m_node->GetId() << ". Sending packet.");

      WifiTxVector txVector= WifiTxVector();
      txVector.SetMode (m_currentWifiMode);
    
      switch (utilitySendMode)
        {
        case WirelessModuleUtility::SEND_AS_JAMMER:
          NS_LOG_DEBUG ("NslWifiPhy: Sending packet as jammer." << packet);
          NS_LOG_INFO ("NslWifiPhy: powerLevel"<<powerLevel);
          txVector.SetPreambleType(WIFI_PREAMBLE_INVALID);
          SendPacket (packet, txVector, WIFI_PREAMBLE_INVALID, powerLevel);
          NS_LOG_INFO ("test state"<<m_state->GetState ());
          break;
        case WirelessModuleUtility::SEND_AS_HONEST:
          {
            WifiMacHeader hdr;
            hdr.SetType (WIFI_MAC_DATA);
            packet->AddHeader (hdr);
            NS_LOG_DEBUG ("NslWifiPhy: Sending packet as honest node.");
            txVector.SetPreambleType(WIFI_PREAMBLE_LONG);
            SendPacket (packet, txVector, WIFI_PREAMBLE_LONG, powerLevel);
          }
          break;
        case WirelessModuleUtility::SEND_AS_OTHERS:
          NS_FATAL_ERROR ("NslWifiPhy:Undefined utility send packet mode!");
          break;
        default:
          break;
        }
      // update the actual TX power
      powerW = DbmToW (WifiPhy::GetTxPowerStart());
      powerW += powerLevel * DbmToW ((WifiPhy::GetTxPowerEnd()  - WifiPhy::GetTxPowerStart()) / WifiPhy::GetNTxPower());
    }
  else
    {
      NS_LOG_DEBUG ("NslWifiPhy: Node # "<< m_node->GetId () <<
                    "Error in send packet callback. Incorrect power level.");
      // set sent power to 0 to indicate error
      powerW = 0;
    }
}

double
NslWifiPhy::DbToRatio (double dB) const
{
  double ratio = pow(10.0,dB/10.0);
  return ratio;
}

double
NslWifiPhy::DbmToW (double dBm) const
{
  double mW = pow(10.0,dBm/10.0);
  return mW / 1000.0;
}

/*double
NslWifiPhy::WToDbm (double w) const
{
  return 10.0 * log10(w * 1000.0);
}*/

/*double
NslWifiPhy::RatioToDb (double ratio) const
{
  return 10.0 * log10(ratio);
}*/

/*double
NslWifiPhy::GetEdThresholdW (void) const
{
  return m_edThresholdW;
}*/

void
NslWifiPhy::SendPacket (Ptr<const Packet> packet,  WifiTxVector txVector,
                        WifiPreamble preamble, uint8_t txPower)
{
  NS_LOG_FUNCTION (this << packet << txVector << preamble << (uint32_t)txPower);
  /*
   * Transmission can happen if:
   *  - we are syncing on a packet. It is the responsibility of the
   *    MAC layer to avoid doing this but the PHY does nothing to
   *    prevent it.
   *  - we are idle
   */
  NS_LOG_FUNCTION ("test1" << m_state->GetState());
  NS_ASSERT (!m_state->IsStateTx () && !m_state->IsStateSwitching ());
  NS_LOG_FUNCTION ("test1" << m_state->GetState());
  NS_LOG_FUNCTION ("test1");
  Time txDuration = CalculateTxDuration (packet->GetSize (), txVector, GetFrequency ());
   NS_LOG_FUNCTION ("test4" <<packet->GetSize());
  if (m_state->IsStateRx ())
    {
     NS_LOG_FUNCTION ("test44");
      m_endRxEvent.Cancel ();
      m_interference.NotifyRxEnd ();
    }
 NotifyTxBegin (packet,txVector.GetTxPowerLevel());
 
    
  NotifyMonitorSniffTx (packet,  GetFrequency (),txVector);
  m_state->SwitchToTx (txDuration, packet, GetPowerDbm (txVector.GetTxPowerLevel ()), txVector);
  NS_LOG_DEBUG ("NslWifiPhy:Calling Send function .. txPower = " <<
                (int) txPower << " in Dbm = " << GetPowerDbm (txPower) <<
                " plus gain = " << WifiPhy::GetTxGain());
  m_channel->Send (this, packet, GetPowerDbm (txPower) + WifiPhy::GetTxGain(),txDuration,preamble,txVector);

  /*
   * Driver interface.
   */
 // SetCurrentWifiMode (txMode.GetMode());
  DriverStartTx (packet, DbmToW (GetPowerDbm (txPower) + WifiPhy::GetTxGain()));
}

void
NslWifiPhy::DriverStartTx (Ptr<const Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);
  // notify utility for start of TX
  if (m_utility != NULL)
    {
      m_utility->StartTxHandler (packet, txPower);
    }
  else
    {
      NS_LOG_DEBUG ("NslWifiPhy:Utility module is *not* installed on Node #" << m_node->GetId ());
    }
  // schedule DriverEndTx
  Simulator::Schedule (m_state->GetDelayUntilIdle (), &NslWifiPhy::DriverEndTx, this, packet, txPower);
}

void
NslWifiPhy::DriverEndTx (Ptr<const Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);

  /*
   * TX can not be interrupted. Hence we do not have to check state within this
   * function.
   */

  // notify utility for end of TX
  if (m_utility != NULL)
    {
      m_utility->EndTxHandler (packet, txPower);
    }
  else
    {
      NS_LOG_DEBUG ("NslWifiPhy:Utility module is *not* installed on Node #" << m_node->GetId ());
    }
}

double
NslWifiPhy::MeasureRss (void)
{
  NS_LOG_FUNCTION (this);
   return m_interference.CurrentNodeRss (m_txVector);
}


void
NslWifiPhy::SetChannelNumber (uint16_t nch)
{
  if (Simulator::Now () == Seconds (0))
    {
      // this is not channel switch, this is initialization
      NS_LOG_DEBUG("start at channel " << nch);
      m_channelNumber = nch;
      return;
    }
    NS_LOG_DEBUG (m_state->GetState ());

  NS_ASSERT(!IsStateSwitching ());
  NS_LOG_DEBUG ("NslWifiPhy:Attempting to set channel number to " << nch);
  switch (m_state->GetState ()) {
  case WifiPhyState::RX:
    NS_LOG_DEBUG ("drop packet because of channel switching while reception");
    m_endRxEvent.Cancel();
    goto switchChannel;
    break;
  case WifiPhyState::TX:
    NS_LOG_DEBUG ("channel switching postponed until end of current transmission");
    Simulator::Schedule (GetDelayUntilIdle(), &NslWifiPhy::SetChannelNumber, this, nch);
    break;
  case WifiPhyState::CCA_BUSY:
  case WifiPhyState::IDLE:
    goto switchChannel;
    break;
  case WifiPhyState::SLEEP:
    NS_LOG_DEBUG ("channel switching ignored in sleep mode");
    break;
  case WifiPhyState::OFF:
    NS_LOG_DEBUG ("channel switching ignored in sleep mode");
    break;
  default:
    NS_ASSERT (false);
    break;
  }

  return;

  switchChannel:

  NS_LOG_DEBUG("NslWifiPhy:switching channel " << m_channelNumber << " -> "
      << nch << ", At time = " << Simulator::Now ().GetSeconds () << "s");

  NS_LOG_DEBUG("NslWifiPhy:switching channel " << m_channelSwitchDelay);
      
  m_state->SwitchToChannelSwitching(m_channelSwitchDelay);
  m_interference.EraseEvents();
  /*
   * Needed here to be able to correctly sensed the medium for the first
   * time after the switching. The actual switching is not performed until
   * after m_channelSwitchDelay. Packets received during the switching
   * state are added to the event list and are employed later to figure
   * out the state of the medium after the switching.
   */
  m_channelNumber = nch;

  /*
   * Driver interface.
   */
  UpdatePhyLayerInfo ();
}

double
NslWifiPhy::WToDbm (double w) const
{
  return 10.0 * log10(w * 1000.0);
}

double
NslWifiPhy::RatioToDb (double ratio) const
{
  return 10.0 * log10(ratio);
}

/*double
NslWifiPhy::GetEdThresholdW (void) const
{
  return m_edThresholdW;
}*/

double
NslWifiPhy::GetPowerDbm (uint8_t power) const
{
  NS_ASSERT ( WifiPhy::GetTxPowerStart() <=  WifiPhy::GetTxPowerEnd());
  NS_ASSERT (WifiPhy::GetNTxPower() > 0);
  double dbm;
  if (WifiPhy::GetNTxPower() > 1)
    {
      dbm =  WifiPhy::GetTxPowerStart() + power * ( WifiPhy::GetTxPowerEnd() -  WifiPhy::GetTxPowerStart()) / (WifiPhy::GetNTxPower() - 1);
    }
  else
    {
      NS_ASSERT_MSG ( WifiPhy::GetTxPowerStart() ==  WifiPhy::GetTxPowerEnd(),
                     "cannot have TxPowerEnd != TxPowerStart with TxPowerLevels == 1");
      dbm =  WifiPhy::GetTxPowerStart();
    }
  return dbm;
}

void
NslWifiPhy::StartReceivePacket (Ptr<Packet> packet,
                                double rxPowerDbm,
                                WifiTxVector txMode,
                                uint16_t preamble)
{
  NS_LOG_FUNCTION (this << packet << rxPowerDbm << txMode << preamble);
  rxPowerDbm += WifiPhy::GetRxGain();
  double rxPowerW = DbmToW (rxPowerDbm);
  Time rxDuration = CalculateTxDuration (packet->GetSize (), txMode, preamble);
  NS_LOG_FUNCTION ( rxDuration);
  Time endRx = Simulator::Now () + rxDuration;

   Ptr<Event> event;
  event = m_interference.Add (packet,
                              txMode,
                              rxDuration,
                              rxPowerW);

  /*
   * Driver interface.
   */
  SetCurrentWifiMode (txMode.GetMode());

  switch (m_state->GetState ()) {
  case WifiPhyState::SWITCHING:
    NS_LOG_DEBUG ("NslWifiPhy:drop packet because of channel switching");
    NotifyRxDrop (packet,NOT_ALLOWED);
    /*
     * Packets received on the upcoming channel are added to the event list
     * during the switching state. This way the medium can be correctly sensed
     * when the device listens to the channel for the first time after the
     * switching e.g. after channel switching, the channel may be sensed as
     * busy due to other devices' transmissions started before the end of the
     * switching.
     */
    if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
      {
        // that packet will be noise _after_ the completion of the
        // channel switching.
        goto maybeCcaBusy;
      }
    break;
  case WifiPhyState::RX:
    NS_LOG_DEBUG ("NslWifiPhy:drop packet because already in Rx (power = "<<
                  rxPowerW<<" W)");
    NotifyRxDrop (packet,NOT_ALLOWED);
    if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
      {
        // that packet will be noise _after_ the reception of the
        // currently-received packet.
        goto maybeCcaBusy;
      }
    break;
  case WifiPhyState::TX:
    NS_LOG_DEBUG ("NslWifiPhy:drop packet because already in Tx (power = "<<
                  rxPowerW<<" W)");
    NotifyRxDrop (packet,NOT_ALLOWED);
    if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
      {
        // that packet will be noise _after_ the transmission of the
        // currently-transmitted packet.
        goto maybeCcaBusy;
      }
    break;
  case WifiPhyState::CCA_BUSY:
  case WifiPhyState::IDLE:
    if (rxPowerW > -140)
      {
        NS_LOG_DEBUG ("NslWifiPhy:SYNC (power = " << rxPowerW << " W) at Node #" <<
                      m_node->GetId ());
        NS_LOG_DEBUG ("preamble" << preamble);
                      
        // drop noise/jamming packet
        if (preamble == WIFI_PREAMBLE_INVALID)
          {
            NS_LOG_DEBUG ("NslWifiPhy:drop **jamming** packet!" << packet);
            NotifyRxDrop (packet,NOT_ALLOWED);
            goto maybeCcaBusy;
          }

        /*
         * Driver interface.
         */
        if (!DriverStartRx (packet, MeasureRss ()))
          {
            NS_LOG_DEBUG ("NslWifiPhy:Ignoring RX! at Node #" << m_node->GetId ());
            NotifyRxDrop (packet,NOT_ALLOWED);
            return; // still in IDLE or CCA_BUSY
          }

        // sync to signal
        m_state->SwitchToRx (rxDuration);
        NS_ASSERT (m_endRxEvent.IsExpired ());
        NotifyRxBegin (packet);
        m_interference.NotifyRxStart ();
        m_endRxEvent = Simulator::Schedule (rxDuration, &NslWifiPhy::EndReceive,
                                            this, packet, event);
      }
    else  // drop because power too low
      {
        NS_LOG_DEBUG ("NslWifiPhy:drop packet because signal power too Small (" <<
                      rxPowerW << "<" << -140 << ")");
        NotifyRxDrop (packet,NOT_ALLOWED);
        goto maybeCcaBusy;
      }
    break;
    case WifiPhyState::SLEEP:
        NS_LOG_DEBUG ("Drop packet because in sleep mode");
        NotifyRxDrop (packet, NOT_ALLOWED);
    break;
    case WifiPhyState::OFF:
     NS_LOG_DEBUG ("Drop packet because in sleep mode");
        NotifyRxDrop (packet, NOT_ALLOWED);
        WifiPhy::ResumeFromOff();
      break;
  
  }
  return;

 maybeCcaBusy:
  // We are here because we have received the first bit of a packet and we are
  // not going to be able to synchronize on it
  // In this model, CCA becomes busy when the aggregation of all signals as
  // tracked by the InterferenceHelper class is higher than the CcaBusyThreshold

   //WifiPhy::SwitchMaybeToCcaBusy();

   Time delayUntilCcaEnd = m_interference.GetEnergyDuration (250);
  if (!delayUntilCcaEnd.IsZero ())
    {
      m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd);
    }  
}

void
NslWifiPhy::EndReceive (Ptr<Packet> packet, Ptr<Event> event)
{
  NS_LOG_FUNCTION (this << packet << event);
  //NS_ASSERT (IsStateRx ());
  NS_ASSERT (event->GetEndTime () == Simulator::Now ());

  /*
   * Make a copy of the original packet to pass to WirelessModuleUtility.
   */
  Ptr<Packet> copy = packet->Copy ();

  struct InterferenceHelper::SnrPer snrPer;
  snrPer = m_interference.CalculateSnrPer (event);
  m_interference.NotifyRxEnd ();
  //WifiTxVector txVector = event->GetTxVector ();
  std::pair<bool, SignalNoiseDbm> rxInfo;
  std::vector<bool> statusPerMpdu;
  Time psduDuration = event->GetEndTime () - event->GetStartTime ();
   Time relativeStart = NanoSeconds (0);


  NS_LOG_DEBUG ("mode=" << (event->GetPayloadMode ().GetDataRate (WifiPhy::GetChannelWidth ())) << ", snr=" <<
                snrPer.snr << ", per=" << snrPer.per << ", size=" << packet->GetSize ());
  
  rxInfo = GetReceptionStatus (packet, event, relativeStart, psduDuration);
  
  if(rxInfo.first){
      NotifyRxEnd (packet);
      NS_LOG_DEBUG(event->GetPreambleType ());
       //bool isShortPreamble = (WIFI_PREAMBLE_SHORT == event->GetPreambleType ());
        //double signalDbm = RatioToDb (event->GetRxPowerW ()) + 30;
       // double noiseDbm = RatioToDb (event->GetRxPowerW () / snrPer.snr) - GetRxNoiseFigure () + 30;
        SignalNoiseDbm noiseDbm2 = rxInfo.second;
        statusPerMpdu.push_back (rxInfo.first);
        NotifyMonitorSniffRx (packet, (uint16_t)GetChannelFrequencyMhz (), event->GetTxVector(), noiseDbm2, statusPerMpdu);
        m_state->SwitchFromRxEndOk (packet, snrPer.snr, event->GetTxVector(),statusPerMpdu);
       
      DriverEndRx (copy, snrPer.packetRss, true);
    }
  else
    {
      /* failure. */
      NS_LOG_DEBUG(event->GetPreambleType ());
      NotifyRxDrop (packet,NOT_ALLOWED);
      m_state->SwitchFromRxEndError (packet, snrPer.snr);
      /*
       * Driver interface.
       */
    
      DriverEndRx (copy, snrPer.packetRss, false);
    }
}

void
NslWifiPhy::DriverEndRx (Ptr<Packet> packet, double averageRssW,
                         const bool isSuccessfullyReceived)
{
  NS_LOG_FUNCTION (this << packet << averageRssW << isSuccessfullyReceived);

  // notify utility for end of RX
  if (m_utility != 0)
    {
      m_utility->EndRxHandler (packet, averageRssW, isSuccessfullyReceived);
    }
  else
    {
      NS_LOG_DEBUG ("NslWifiPhy:Utility module is *not* installed on Node #" << m_node->GetId ());
    }
}


bool
NslWifiPhy::DriverStartRx (Ptr<Packet> packet, double startRssW)
{
  
  NS_LOG_FUNCTION (this << packet << startRssW);

  bool isPacketToBeReceived = true;

  // notify utility for start of RX
  if (m_utility != NULL)
    {
      isPacketToBeReceived =  m_utility->StartRxHandler (packet, startRssW);
    }
  else
    {
      NS_LOG_DEBUG ("NslWifiPhy:Utility module is *not* installed on Node #" << m_node->GetId ());
    }

     
    Simulator::Schedule (m_state->GetDelayUntilIdle (), &NslWifiPhy::DriverEndTx, this, packet, startRssW);

  return isPacketToBeReceived;
}


} //namespace ns3
