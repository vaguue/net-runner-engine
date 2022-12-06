const Simulator = require('../..');
const path = require('path');
const fs = require('fs');

const { Network, Hub, Switch, Host, TCPClient, TCPServer, UDPClient, UDPServer } = Simulator;

const dstDir = path.resolve(__dirname, 'files/routing');

const net = new Network({ 
  animeLen: 1,
  populateIP: true,
});

const host1 = new Host(net, { name: 'UdpClient', x: 0, y: 0 });
const host2 = new Host(net, { name: 'UdpServer', x: 400, y: 0 });

host1.setupApplication(new UDPClient({ 
  addr: '192.168.2.1', 
  port: 3000,
  onTick: ({ time, sendPacket }) => {
    if (time < 1000) {
      const buf = Buffer.from('hello');
      sendPacket(buf);
    }
    return '0.01s';
  },
}));

host2.setupApplication(new UDPServer({ 
  port: 3000,
  onReceive: ({ address, packet, reply }) => {
    console.log('[*] receive', address, packet);
    const buf = Buffer.from('world?');
    reply(buf);
  },
}));

const swtch = new Switch(net, { name: 'MyHomeSwitch', x: 800, y: 0 });
const tmpHosts = Array.from({ length: 3 }, (_, i) => new Host(net, { name: `Host${i}`, y: 200, x: 800 + i*200 }));

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

const isMain = require.main === module;

async function main() {
  if (isMain) {
    return await net.run(dstDir, { upload: true }).then(url => console.log('[*] uploaded', url));
  }
  return net.run(dstDir);
}

module.exports = main;

if (isMain) {
  main().catch(console.error);
}
