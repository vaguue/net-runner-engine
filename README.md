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

const { fromConfig, Network, Hub, Switch, Host, TCPClient, TCPServer, UDPClient, UDPServer } = Simulator;

const dstDir = path.resolve(__dirname, 'files');
if (!fs.existsSync(dstDir)) {
  fs.mkdirSync(dstDir);
}

const net = new Network({ 
  animeLen: 5, // seconds, default is 10
});
const host1 = new Host({ 
  name: 'Alice', // this name will be used in resulting PCAP files, optional (some numbers will be used if not specified, e.g. 0-1-hub-csma-0.pcap)
});
const host2 = new Host({ name: 'Bob' });
host1.setupApplication(new TCPClient({ 
  dst: '192.168.1.3:3000', // accepts dst in format <IP address>:<port>
}));
host2.setupApplication(new TCPServer({ 
  dst: '3000', // accepts only port number via dst field,
}));
// each host should be added to network BEFORE conneting
net.addNode(host1); 
net.addNode(host2);
// connecting two hosts
host1.connect(host2, { 
  sourceIP: '192.168.1.2', //IP of host1's device, required for Host node
  targetIP: '192.168.1.3', //IP of host2's device, required for Host node
  dataRate: '1Mbps', //optional, default is 5Mbps
  delay: '1ms', //optional, default is 5ms
});

//Openflow switch example
const swtch = new Switch({ name: 'MyHomeSwitch' });
net.addNode(swtch);
const n = 4;
const half = Math.floor(n / 2);
const hosts = [...Array(n).keys()].map((_, i) => new Host({ name: `user${i + 1}` }));
for (let i = 0; i < n; ++i) {
  net.addNode(hosts[i]);
  hosts[i].connect(swtch, {
    sourceIP: `192.168.0.${i + 1}`, //NOTE: Switch does not have IP address, only host
  });
}
//setting up UDP clients and servers
for (let i = 0; i < half; ++i) {
  hosts[i].setupApplication(new UDPClient({ dst: `192.168.0.${i + half}:8888` }));
  hosts[n - i - 1].setupApplication(new UDPServer({ dst: '8888' }));
}
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
