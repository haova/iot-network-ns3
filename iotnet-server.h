#ifndef IOTNET_SERVER_H
#define IOTNET_SERVER_H

#include "ns3/application.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/stats-module.h"

using namespace ns3;

/**
 * IotNetServer application.
 */
class IotNetServer : public Application
{
  public:
    /**
     * \brief Get the type ID.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();

    IotNetServer();
    ~IotNetServer() override;

    /**
     * Set the counter calculator for received packets.
     * \param calc The CounterCalculator.
     */
    void SetCounter(Ptr<CounterCalculator<>> calc);

    /**
     * Set the delay tracker for received packets.
     * \param delay The Delay calculator.
     */
    void SetDelayTracker(Ptr<TimeMinMaxAvgTotalCalculator> delay);

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * Receive a packet.
     * \param socket The receiving socket.
     */
    void Receive(Ptr<Socket> socket);

    Ptr<Socket> m_socket; //!< Receiving socket
    uint32_t m_port{0};   //!< Listening port

    Ptr<CounterCalculator<>> m_calc;           //!< Counter of the number of received packets
    Ptr<TimeMinMaxAvgTotalCalculator> m_delay; //!< Delay calculator
};

#endif /* IOTNET_SERVER_H */
