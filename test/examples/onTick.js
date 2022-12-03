const Simulator = require('../..');
const path = require('path');
const fs = require('fs');

const { Network, Hub, Switch, Host, TCPClient, TCPServer, UDPClient, UDPServer } = Simulator;

const dstDir = path.resolve(__dirname, 'files/on-tick');

const net = new Network({ 
  animeLen: 5, // seconds, default is 10
});

const host1 = new Host({ 
  name: 'Alice', // this name will be used in resulting PCAP files, optional (some numbers will be used if not specified, e.g. 0-1-hub-csma-0.pcap)
});

const host2 = new Host({ name: 'Bob' });
host1.setupApplication(new TCPClient({ 
  addr: '192.168.1.3',
  port: 3000,
  onTick: ({ time, sendPacket }) => { // you can implement you custom logic here
    if (time > 1000) {
      const buf = Buffer.from('hello');
      sendPacket(buf); //accepts Buffer only
    }
    return '1s'; //call onTick after 0.1s
  },
}));

host2.setupApplication(new TCPServer({ 
  port: 3000,
  onReceive: ({ address, packet, reply }) => { // custom receive callback
    console.log('[*] receive', address, packet);
    const buf = Buffer.from('world?');
    reply(buf);
  },
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

function main() {
  return net.run(dstDir);
}

module.exports = main;

if (require.main === module) {
  main();
}
