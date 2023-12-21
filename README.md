# IoTNet

Simulate Jamming in IoT Network using NS3.

## Information

- Course: NT2203.CH1702
- Members:
  - 220202014 - Vũ Anh Hào
  - 230202006 - Ngô Thái Hưng
  - 220202020 - Nguyễn Duy Minh Nhật
  - 230202009 - Nguyễn Tấn Kiệt

## Directory Structure

```txt
$
|- ns3 (Network Simulation)
|- next-app (Web Application Server)
|- sensor-node (Sensor Node Firmware)
```

## Prerequisites

### Hardware

- Wifi SoC ESP8266 V12
- USB UART CP2102

### Software

- Docker CE
- Docker Compose

## Setup

```bash
docker compose up
```

## Simulation Network

### Guide

Access docker container.

```bash
docker exec -it iotnet-ns3 /bin/bash
```

Run IoT Network

```bash
./waf --run scratch/iotnet/iotnet
```

or

```bash
docker exec -it iotnet-ns3 /root/ns-allinone-3.30.1/ns-3.30.1/waf --run scratch/iotnet/iotnet
```

### Classes

**IoTNetWifi**

Định nghĩa một mạng wifi nội bộ.

## Web Application Server

Run Real Server

```bash
docker exec -it server-node /bin/bash
npm i
npm run dev
```

## Tshark

```bash
tshark -r output/iotnet-0-0.pcap
tshark -q -r output/iotnet-0-0.pcap -z follow,tcp,ascii,0
```

## ESP8266

### Devices

## Checklist

- [x] Sensor node can send a TCP packet.
- [x] Server node can received a TCP packet.
- [x] Sensor node can read rssi, snr.
- [x] Sensor node should prepare data for send.
- [x] Server node can forward packet to outside.
- [x] PDR - Thiết bị thật (Kiệt)
- [ ] PDR - Giả lập (-)
- [-] Jammer - Thiết bị thật (Hưng - deauth, Hào)
- [ ] Jammer - Giả lập - source code https://codereview.appspot.com/1055041/#ps321001 (Hưng)
- [x] Model detection - (Nhật)
  - source code 1: https://github.com/kasturi710/Jamming-Attack-Detection-and-Classification
  - source code 2: https://github.com/AMHD/Jamming-Detection-in-IoT-Wireless-Networks-An-Edge-AI-Based-Approach
- [ ] Implement model detection into next-app (Nhật)
- [ ]
