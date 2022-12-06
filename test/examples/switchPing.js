const Simulator = require('../..');
const path = require('path');
const fs = require('fs');

const { Network, Hub, Switch, Host, Ping } = Simulator;

const dstDir = path.resolve(__dirname, 'files/switch-ping');

const net = new Network({ 
  animeLen: 5,
  populateIP: true,
});

const host1 = new Host({ name: 'Alice', x: 0, y: 0 });
const host2 = new Host({ name: 'Bob', x: 600, y: 0 });
const host3 = new Host({ name: 'Chill', x: 1200, y: 0 });

const swtch = new Switch({ name: 'Switch', x: 300, y: 400 });

net.addNode(host1);
net.addNode(host2);
net.addNode(host3);
net.addNode(swtch);

host1.connect(swtch, {
  sourceIP: '192.168.1.1',
});

host2.connect(swtch, {
  sourceIP: '192.168.1.2',
});

host3.connect(swtch, {
  sourceIP: '192.168.1.3',
});

host1.setupApplication(new Ping({
  addr: '192.168.1.2',
  interval: '0.5s',
}));

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
