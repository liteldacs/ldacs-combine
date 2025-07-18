#!/bin/bash

ldb_path="/home/jiaxv/ldacs/ldacs-combine/cmake-build-debug/bin"

cb-as() {
  if [[ ! "$1" =~ ^[0-9]+$ ]]; then
    echo "错误：请提供一个正整数参数（例如：cb-as 1）"
    return 1
  fi
  echo "clear && ./ldacs-combine -c \"../../config/ldacs_config_as_$1.yaml\""
}

for i in {1..30}; do

  cmd=$(cb-as $i)
  echo $cmd

#  gnome-terminal -- bash -l -c \
#    "cd '$ldb_path' && \
#     $cmd && \
#     exec bash" &
#
#
  konsole  --geometry 1000x500--hold -e bash -l -c \
         "cd '$ldb_path' && \
          $cmd && \
          exec bash" &
  sleep 2
done