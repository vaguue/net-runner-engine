# Net-runner-engine


<p align="center">
  <img alt="Net-runner" src="https://raw.githubusercontent.com/hedonist666/net-runner-engine/main/logo.svg" height="420" width="620"/>
</p>


## About
This is the [NS-3](https://www.nsnam.org/) based [node-addon-api](https://github.com/nodejs/node-addon-api) module used for the [Net-runner](https://net-runner.xyz) website. 
It's core functionality is to launch the NS-3 functions according to JSON config specified as an argument and dump the results. Example config and usage from Node.js can be found at [test](https://github.com/hedonist666/net-runner-engine/blob/main/test/test_binding.js) folder.
## Goal
The goal of this project is to provide the web-based platform who those who are learning the inner workings of computer networks. Although the project is up and running, this project is very young and has **many** work to do. So I decided to do this with the NS-3 community all together!
If you are an NS-3 expert, or Node.js enthusiast, or just want to commit to this project, feel free to contact me (see contacts below) and ask any questions.
## Installation
There is dedicated Docker [image](https://hub.docker.com/r/netrunnerxyz/ns3-node) for installing and using this module. It you want to install this module in your system, just run 
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

const { fromConfig, Network, Hub, Switch, Host, TCPClient, TCPServer } = Simulator;

const dstDir = path.resolve(__dirname, 'files');
if (!fs.existsSync(dstDir)) {
  fs.mkdirSync(dstDir);
}

const net = new Network();
const hosts = [...Array(3).keys()].map((_, i) => new Host({ x: 100*i, y: 100*i })).map(e => net.addNode(e));
const hub = new Hub({ x: 0, y: 100 });
net.addNode(hub);
hosts.map((e, i) => e.connect(hub, { sourceIP: `192.168.1.${i}`, targetIP: `192.168.1.${hosts.length + i}` }));
hosts[0].setupApplication(new TCPClient({ dst: '192.168.1.2', port: '3000' }));
hosts[2].setupApplication(new TCPServer({ port: '3000' }));
net.run(dstDir);
```
After running this script you'll see appropriate PCAP files for each interface of each network's node in 'files' directory.
## Architecture
The structure of internal config is shown below:
```
{
  nodes: [{ id, title, x, y, type, applications }],
  edges: [{ source, target, type, sourceIP, targetIP }],
  options: { animeLen: 10, popuplateIP: true }
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
- [ ] Routing table dumps 
- [ ] Connection configuration (channel attributes)
- [ ] Setting up IP/MAC addresses manually
- [ ] DHCP server functionality
- [ ] WIFI
- [ ] TCP configuration (window, different stacks)
- [ ] IPIP, PPPoE, VPN (GRE)
## Contacts
Email: sabrinarempel7@gmail.com
Feel free to ask any questions!
