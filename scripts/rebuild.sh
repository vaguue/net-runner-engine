#!/bin/bash

#RUN IN DOCKER
root=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd );
cd $root/.. 

node-gyp --debug configure
node-gyp --debug build
