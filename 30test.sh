#!/bin/bash

ldb_path="/home/jiaxv/ldacs/ldacs-combine/cmake-build-debug/bin"

cb-as() {
  if [[ ! "$1" =~ ^[0-9]+$ ]]; then
    echo "错误：请提供一个正整数参数（例如：cb-as 1）"
    return 1
  fi
  echo "clear && ./ldacs-combine -c \"../../config/ldacs_config_as_$1.yaml\""
}

#for i in {1..30}; do
#
#  cmd=$(cb-as $i)
#  echo $i $cmd
#
#  konsole  --geometry 1000x500 --hold -e bash -l -c \
#         "cd '$ldb_path' && \
#          $cmd && \
#          exec bash" &
#  sleep 0.2
#done
for i in {1..12}; do

  new_i1=$((i * 2 - 1))
  new_i2=$((i * 2 ))
  cmd1=$(cb-as $new_i1)
  cmd2=$(cb-as $new_i2)

  echo $new_i1 $cmd1
  konsole  --geometry 1000x500 --hold -e bash -l -c \
         "cd '$ldb_path' && \
          $cmd1 && \
          exec bash" &

  echo $new_i2 $cmd2
  konsole  --geometry 1000x500 --hold -e bash -l -c \
         "cd '$ldb_path' && \
          $cmd2 && \
          exec bash" &

  sleep 0.24
done
