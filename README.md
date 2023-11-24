# IoTNet

Simulate Jamming in IoT Network using NS3.

## Information

- Course: NT2203.CH1702
- Members:
  - 220202014 - Vũ Anh Hào
  - 230202006 - Ngô Thái Hưng
  - 220202020 - Nguyễn Duy Minh Nhật
  - 230202009 - Nguyễn Tấn Kiệt

## Installation

Extract `ns-allinone-3.40.tar.bz2`.

Then, clone this repository:

```bash
cd ns-allinone-3.40/ns-3.40/contrib
git clone git@github.com:haova/iot-network-ns3.git iotnet
cd ../
```

Next, run commands to build modules.

```bash
./ns3 configure --enable-examples --enable-tests --enable-python-bindings
./ns3 build
```

Now, you can run an example with `iotnet` module.

```bash
./ns3 run contrib/iotnet/examples/iotnet-example
```
