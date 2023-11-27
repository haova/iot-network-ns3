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

```bash
sudo docker exec -it iotnet-ns3 /bin/bash
```

```bash
./ns3 run examples/tutorial/second
```

## Simulation

Next, run commands to build modules.

```bash
./ns3 configure --enable-examples --enable-tests --enable-python-bindings
./ns3 build
```

Now, you can run an example with `iotnet` module.

```bash
ln -s ../contrib/iotnet/examples/iotnet-example.cc scratch/iotnet-example.cc
./ns3 run scratch/iotnet-example
```
