#!/bin/sh
# increase the queue limits to store up to 5 image frames size 640x480x3
# instead of doing this manually - consider placing this line in
# /etc/sysctl.conf
sudo sysctl -w kernel.msgmax=4608000
sudo sysctl -w kernel.msgmnb=4608000
