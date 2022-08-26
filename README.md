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
## Running
The code is designed to run inside the Docker container, though all necessary node-gyp configs and installation scripts for usage without Docker will be added soon to this repo. Example scripts for running this module can be found at [scripts](https://github.com/hedonist666/net-runner-engine/blob/main/scripts) folder.
## Architecture
The structure of input config is shown below:
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
