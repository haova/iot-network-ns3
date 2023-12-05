# IoTNet

Simulate Jamming in IoT Network using NS3.

## Information

- Course: NT2203.CH1702
- Members:
  - 220202014 - Vũ Anh Hào
  - 230202006 - Ngô Thái Hưng
  - 220202020 - Nguyễn Duy Minh Nhật
  - 230202009 - Nguyễn Tấn Kiệt

## Prerequisites

- Docker CE
- Docker Compose

## Setup

```bash
sudo docker compose up
```

Access docker container.

```bash
sudo docker exec -it iotnet-ns3 /bin/bash
```

Run example

```bash
./ns3 run examples/tutorial/second
```

Run IoT Network

```bash
./ns3 run scratch/iotnet/main
```

Run Real Server

```bash
sudo docker exec -it server-node /bin/bash
npm i
npm run dev
```

## Tshark

```bash
tshark -r output/iotnet-0-0.pcap
tshark -q -r output/iotnet-0-0.pcap -z follow,tcp,ascii,0
```

## Checklist

- [x] Sensor node can send a TCP packet.
- [x] Server node can received a TCP packet.
- [x] Sensor node can read rssi, snr.
- [ ] Sensor node should prepare data for send.
- [x] Server node can forward packet to outside.
