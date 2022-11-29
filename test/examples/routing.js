const Simulator = require('../..');
const path = require('path');
const fs = require('fs');

const { Network, Hub, Switch, Host, TCPClient, TCPServer, UDPClient, UDPServer } = Simulator;

const dstDir = path.resolve(__dirname, 'files/routing');

const net = new Network({ 
  animeLen: 3,
});

const host1 = new Host({ name: 'UdpClient', x: 0, y: 0 });
const host2 = new Host({ name: 'UdpServer', x: 400, y: 0 });

host1.setupApplication(new UDPClient({ 
  dst: '192.168.2.1:3000', 
  onTick: ({ time, sendPacket, tick }) => {
    if (time < 1000) {
      const buf = Buffer.from("hello\0");
      sendPacket(buf);
    }
    tick('0.1s');
  },
}));

host2.setupApplication(new UDPServer({ 
  dst: 3000,
  onRecieve: ({ address, packet, reply }) => {
    console.log('[*] recieve', address, packet);
    const buf = Buffer.from("world?");
    reply(buf);
  },
}));

net.addNode(host1); 
net.addNode(host2);

const swtch = new Switch({ name: 'MyHomeSwitch', x: 800, y: 0 });
const tmpHosts = Array.from({ length: 3 }, (_, i) => new Host({ name: `Host${i}`, y: 200, x: 800 + i*200 }));

net.addNode(swtch);
tmpHosts.forEach(e => net.addNode(e));

host2.connect(swtch, {
  sourceIP: '192.168.2.1',
});

tmpHosts.forEach((e, i) => {
  e.connect(swtch, {
    sourceIP: `192.168.2.${i + 2}`
  });
});

host1.connect(tmpHosts[0], {
  sourceIP: '192.168.1.1',
  targetIP: '192.168.1.2',
});

async function main() {
  return await net.run(dstDir, { upload: true }).then(url => console.log('[*] uploaded', url));
}

module.exports = main;

if (require.main === module) {
  main().catch(console.error);
}
