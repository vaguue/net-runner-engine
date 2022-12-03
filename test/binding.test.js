const Simulator = require('..');
const path = require('path');
const fs = require('fs');
const exampleConfig = require('./rawConfig');

const dstDir = path.resolve(__dirname, 'files');
if (!fs.existsSync(dstDir)) {
  fs.mkdirSync(dstDir);
}
const { fromConfig, Network, Host, Switch, Hub, TCPClient, TCPServer } = Simulator;

test('Running from correct config does not throw', () => {
  expect(() => fromConfig({ pcapPath: dstDir, config: exampleConfig })).not.toThrow();
});

test('Creating network does not throw', () => {
  expect(() => new Network()).not.toThrow();
});

test('Adding nodes', () => {
  const net = new Network();
  net.addNode(new Switch());
  net.addNode(new Host());
  net.addNode(new Hub());
  expect(net.config.nodes.length).toBe(3);
});

test('Removing nodes', () => {
  const net = new Network();
  const hub = new Hub();
  net.addNode(hub);
  expect(net.config.nodes.length).toBe(1);
  net.removeNode(hub);
  expect(net.config.nodes.length).toBe(0);
});

test('Can launch after adding nodes', () => {
  const net = new Network();
  net.addNode(new Switch());
  net.addNode(new Host());
  net.addNode(new Hub());
  expect(() => net.run(dstDir)).not.toThrow();
});

test('Can connect nodes', () => {
  const net = new Network();
  const host1 = new Host();
  const host2 = new Host();
  net.addNode(host1);
  net.addNode(host2);
  host1.connect(host2);
  expect(net.config.nodes.length).toBe(2);
  expect(net.config.edges.length).toBe(1);
  expect(net.config.edges[0].source).toBe(host1.id);
  expect(net.config.edges[0].target).toBe(host2.id);
});

test('Can disconnect nodes', () => {
  const net = new Network();
  const host1 = new Host();
  const host2 = new Host();
  net.addNode(host1);
  net.addNode(host2);
  host1.connect(host2);
  host1.disconnect(host2);
  expect(net.config.nodes.length).toBe(2);
  expect(net.config.edges.length).toBe(0);
});

test('Can launch after adding and nodes', () => {
  const net = new Network();
  const host1 = new Host();
  const host2 = new Host();
  net.addNode(host1);
  net.addNode(host2);
  host1.connect(host2, { sourceIP: '192.168.1.2', targetIP: '192.168.1.3' });
  expect(() => net.run(dstDir)).not.toThrow();
});

test('Can launch multiple networks', () => {
  const net1 = new Network();
  const net2 = new Network();
  expect(() => net1.run(dstDir)).not.toThrow();
  expect(() => net2.run(dstDir)).not.toThrow();
});

test('Can add applications', () => {
  const host = new Host();
  host.setupApplication(new TCPClient({ addr: '192.168.1.3', port :3000 }));
  expect(host.applications.length).toBe(1);
});

test('Can add applications and launch', () => {
  const net = new Network();
  const host1 = new Host();
  const host2 = new Host();
  host1.setupApplication(new TCPClient({ addr: '192.168.1.3', port: 3000 }));
  host2.setupApplication(new TCPServer({ port: 3000 }));
  net.addNode(host1);
  net.addNode(host2);
  host1.connect(host2, { sourceIP: '192.168.1.2', targetIP: '192.168.1.3' });
  expect(() => net.run(dstDir)).not.toThrow();
});

test('Can run verbosely', () => {
  const net = new Network();
  net.setOptions({ verbose: true });
  expect(() => net.run(dstDir)).not.toThrow();
});

test('Can run with custom delay and data rate', () => {
  const net = new Network();
  const host1 = new Host();
  const host2 = new Host();
  host1.setupApplication(new TCPClient({ addr: '192.168.1.3', port: 3000 }));
  host2.setupApplication(new TCPServer({ port: 3000 }));
  net.addNode(host1);
  net.addNode(host2);
  host1.connect(host2, { sourceIP: '192.168.1.2', targetIP: '192.168.1.3', dataRate: '10Mbps', delay: '5ms' });
  expect(() => net.run(dstDir)).not.toThrow();
});

afterAll(done => {
  done();
});
