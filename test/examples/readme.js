const Simulator = require('../..');
const path = require('path');
const fs = require('fs');

const { Network, Hub, Switch, Host, TCPClient, TCPServer, UDPClient, UDPServer } = Simulator;

const dstDir = path.resolve(__dirname, 'files/readme');

const net = new Network({ 
  animeLen: 5, // seconds, default is 10
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
      const buf = Buffer.from("hello");
      sendPacket(buf); //accepts Buffer only
    }
    return '0.1s'; //call onTick after 0.1s
  },
}));

host2.setupApplication(new TCPServer({ 
  port: 3000,
  onReceive: ({ address, packet, reply }) => { // custom receive callback
    console.log('[*] receive', address, packet);
    const buf = Buffer.from("world?");
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

const isMain = require.main === module;

async function main() {
  if (isMain) {
    //simulate network and upload results to http://net-runner.xyz
    return await net.run(dstDir, { upload: true }).then(url => console.log('[*] uploaded', url)); 
  }
  return net.run(dstDir);
}

module.exports = main;

if (isMain) {
  main().catch(console.error);
}
