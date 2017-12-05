#!/bin/bash

/etc/init.d/networking restart		# reload default configurations
ifconfig eth0 up					# activate interface eth0
ifconfig eth0 172.16.50.254/24		# identify interface eth0 (172.16.50.254), mask with 24 bits
ifconfig eth1 up					# activate interface eth1
ifconfig eth1 172.16.51.253/24		# identify interface eth1 (172.16.51.253)
route add default gw 172.16.51.254	# add Rc as default gayeway of tuxy2

echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

# PARTE 5 - DNS
echo "search netlab.fe.up.pt" > /etc/resolv.conf
echo "nameserver 172.16.1.1" >> /etc/resolv.conf
