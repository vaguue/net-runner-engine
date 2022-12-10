# Net-runner-engine [![GitHub license](https://img.shields.io/github/license/vaguue/net-runner-engine?style=flat)](https://github.com/vaguue/net-runner-engine/blob/main/LICENSE) [![npm](https://img.shields.io/npm/v/net-runner-engine)](https://www.npmjs.com/package/net-runner-engine) [![Downloads](https://img.shields.io/npm/dm/net-runner-engine.svg)](http://npm-stat.com/charts.html?package=net-runner-engine) [![Coverage Status](https://coveralls.io/repos/github/vaguue/net-runner-engine/badge.svg)](https://coveralls.io/github/vaguue/net-runner-engine)


<p align="center">
  <img alt="Net-runner" src="https://raw.githubusercontent.com/vaguue/net-runner-engine/main/logo.svg" height="420" width="620"/>
</p>


## About
This is the [NS-3](https://www.nsnam.org/) based [node-addon-api](https://github.com/nodejs/node-addon-api) module used for the [Net-runner](https://net-runner.xyz) website. 
It's core functionality is to launch the NS-3 functions according to Javascript object specified as an argument and dump the results. Example config and usage from Node.js can be found at the [examples](https://github.com/vaguue/net-runner-engine/blob/main/test/examples) folder.
## Goal
The goal of this project is to provide the web-based platform who those who are learning the inner workings of computer networks. Although the project is up and running, this project is very young and has **many** work to do. So I decided to do this with the NS-3 community all together!
If you are an NS-3 expert, or Node.js enthusiast, or just want to commit to this project, feel free to contact me (see contacts below) and ask any questions.
## Installation
There is a dedicated Docker [image](https://hub.docker.com/r/netrunnerxyz/ns3-node) for installing and using this module. It you want to install this module in your system, just run 
```
npm install net-runner-engine
```
However, not all systems are supported. Current supported systems:
* Linux x64
## Standalone usage
Example usage script:
```js
const Simulator = require('net-runner-engine');
const path = require('path');
const fs = require('fs');

const { fromConfig, Network, Hub, Switch, Host, TCPClient, TCPServer, UDPClient, UDPServer } = Simulator;

const dstDir = path.resolve(__dirname, 'files');

const net = new Network({ 
  animeLen: 1, // seconds, default is 3
});

const host1 = new Host(net, { 
  name: 'Alice', // this name will be used in resulting PCAP files, optional (some numbers will be used if not specified, e.g. 0-1-hub-csma-0.pcap)
});
const host2 = new Host(net, { name: 'Bob' });

host1.setupApplication(new TCPClient({ 
  addr: '192.168.1.3',
  port: 3000,
  onTick: ({ time, sendPacket, tick }) => { // you can implement you custom logic here
    if (time > 1000) {
      const buf = Buffer.from('hello');
      sendPacket(buf); //accepts Buffer only
    }
    return '0.01s'; //call onTick after 0.1s
  },
}));

host2.setupApplication(new TCPServer({ 
  port: 3000,
  onReceive: ({ address, packet, reply }) => { // custom recieve callback
    console.log('[*] recieve', address, packet);
    const buf = Buffer.from('world?');
    reply(buf);
  },
}));

// connecting two hosts
host1.connect(host2, { 
  sourceIP: '192.168.1.2', //IP of host1's device, required for Host node
  targetIP: '192.168.1.3', //IP of host2's device, required for Host node
  dataRate: '1Mbps', //optional, default is 5Mbps
  delay: '1ms', //optional, default is 5ms
});

net.run(dstDir, { upload: true }).then(url => console.log('[*] uploaded', url)); //simulate network and upload results to https://net-runner.xyz/
```
After running this script you'll see appropriate PCAP files for each interface of each network's node in 'files' directory.
## Documentation
Documentation for this module can be found [here](https://www.net-runner.xyz/blog/M2w2RJCxrg)
## Architecture
The structure of internal config is shown below:
```
{
  nodes: [{ id, title, x, y, type, applications }],
  edges: [{ source, target, type, sourceIP, targetIP }],
  options: { animeLen: 10, popuplateIP: false }
}
```
A network is represented as a graph that contains information about nodes and connections in a network. The module's work is to:
1. Convert node-addon-api arguments to C++ STL structures.
2. Instantiate network nodes.
3. Set up connections between nodes.
4. Set up NS-3 helpers.
5. Run the simulator.
Steps 2-5 are done using the respective NS-3 functions.
## TODO's
Some tasks are marked done, which means you can use appropriate functionality on the website, though non of them are finished completely, every task is in active state.
- [x] Host node
- [x] Switch node
- [x] Hub node
- [x] ICMP client
- [x] TCP client/server
- [x] UDP client/server
- [x] PCAP dumps (each interface)
- [x] ARP table dumps(each host)
- [x] Connection configuration (channel attributes)
- [x] Documentation
- [x] Routing table dumps 
- [ ] Setting up IP/MAC addresses manually
- [ ] DHCP server functionality
- [ ] WIFI
- [ ] TCP configuration (window, different stacks)
- [ ] IPIP, PPPoE, VPN (GRE)
## Contacts
Email: sabrinarempel7@gmail.com
Feel free to ask any questions!
