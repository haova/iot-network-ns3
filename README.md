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
|- esp8266 (Jamming with deauth in real devices)
|- mlserver (Jamming detection model server)
|- netanim (Network Simulation Animator)
|- next-app (Web Application Server)
|- ns3 (Network Simulation)
|- sensor-node (Sensor Node Firmware for real devices)
```

## Prerequisites

- Docker CE
- Docker Compose

```bash
sudo apt-get install mercurial
sudo apt install g++ python3 cmake ninja-build git
sudo apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
```

## Setup

```bash
docker compose up
```

Sau khi các container chạy thành công, mở `http://localhost:3000/` để theo dõi hệ thống hoạt động.
