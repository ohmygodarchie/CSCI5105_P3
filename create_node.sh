#!/bin/sh

read -p "Enter the IP address of the node: " node_ip
read -p "Enter the port number of the node: " node_port
read -p "Enter the directory you want to share: " share_dir

echo "Creating node with IP address: $node_ip and port number: $node_port"

./communicate_server $node_ip $node_port $share_dir & ./communicate_client $node_ip $node_port $share_dir && fg