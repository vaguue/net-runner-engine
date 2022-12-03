const Simulator = require('../..');
const path = require('path');
const fs = require('fs');

const { Network, Hub, Switch, Host, Ping } = Simulator;

const dstDir = path.resolve(__dirname, 'files/ping');

const net = new Network({ 
  animeLen: 5, // seconds, default is 10
});

const host1 = new Host({ name: 'Pinger' });
const host2 = new Host({ name: 'PingTarget' });

host1.setupApplication(new Ping({ addr: '192.168.1.3', interval: '1s' }));

net.addNode(host1); 
net.addNode(host2);

host1.connect(host2, { 
  sourceIP: '192.168.1.2',
  targetIP: '192.168.1.3',
  dataRate: '1Mbps',
  delay: '1ms',
});

function main() {
  return net.run(dstDir);
}

module.exports = main;

if (require.main === module) {
  main();
}
