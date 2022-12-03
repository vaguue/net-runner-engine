const Simulator = require('../..');
const path = require('path');
const fs = require('fs');

const { Network, Hub, Switch, Host, TCPClient, TCPServer, UDPClient, UDPServer } = Simulator;

const dstDir = path.resolve(__dirname, 'files/switch');

const net = new Network({ 
  animeLen: 5, // seconds, default is 10
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
  hosts[i].setupApplication(new UDPClient({ 
    addr: `192.168.0.${i + half}`, 
    port: 8888,
    dataRate: '1Mbps', // without onTick provided, application will just generate traffic, default data rate is 5Mbps
  }));
  hosts[n - i - 1].setupApplication(new UDPServer({ 
    port: 8888 // here you could provide onReceive, but this is optional
  }));
}

function main() {
  return net.run(dstDir);
}

module.exports = main;

if (require.main === module) {
  main();
}
