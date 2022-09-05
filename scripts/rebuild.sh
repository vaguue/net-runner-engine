#!/bin/bash

#RUN IN DOCKER
root=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd );
cd $root/.. 

#./scripts/run.sh node-gyp --debug configure
#./scripts/run.sh node-gyp --debug build

#./scripts/run.sh npm i

./scripts/run.sh node-gyp rebuild
