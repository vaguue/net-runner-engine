#!/bin/bash

resolve() {
  cd "$1" || return 1
  pwd
}
root=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd );
d=$(resolve $root/..)

docker run --env LD_LIBRARY_PATH=/root/ns-allinone-3.29/bake/build/lib -v $d/:/root/wrapper -w /root/wrapper -it ns3Docker bash
