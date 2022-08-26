#!/bin/bash

root=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd );
cd $root/.. 

docker build -t ns3Docker - < Dockerfile
