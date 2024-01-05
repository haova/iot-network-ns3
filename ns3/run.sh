#!/bin/sh

cd /root/ns-allinone-3.30.1/ns-3.30.1
./waf --run "scratch/iotnet/iotnet --duration=120 --realtime --jamming"