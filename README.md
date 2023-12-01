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
sudo docker exec -it server-node /bin/bash
```

```bash
./ns3 run examples/tutorial/second
./ns3 run scratch/iotnet/main
```

## Tshark

```bash
tshark -r output/iotnet-0-0.pcap
tshark -q -r output/iotnet-0-0.pcap -z follow,tcp,ascii,0
```

## Checklist

- [x] Sensor node can send a TCP packet.
- [x] Server node can received a TCP packet.
- [ ] Sensor node can read rssi, snr.
- [ ] Server node can forward packet to outside.