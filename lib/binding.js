const path = require('path');
const fs = require('fs');
const os = require('os');
const { createGzip } = require('zlib');
const tar = require('tar-fs');
const pipeline = require('util').promisify(require('stream').pipeline);
const stream = require('stream');

const { Application, ...Applications } = require('./applications');
const { Node, ...Nodes } = require('./nodes');
const { validate, autoGen } = require('./configFactory');

const binPath = '../build/Release/wrapper-native';
const { Instance } = require(binPath);
const bindings = new Instance();

const { v4: uuidv4 } = require('uuid');

require('dotenv').config({ path: path.resolve(__dirname, '..', '.env') });

const url = process.env.BACKEND || 'https://emu.net-runner.xyz/config/record';

function streamToString(stream) {
  const chunks = [];
  return new Promise((resolve, reject) => {
    stream.on('data', (chunk) => chunks.push(Buffer.from(chunk)));
    stream.on('error', (err) => reject(err));
    stream.on('end', () => resolve(Buffer.concat(chunks).toString('utf8')));
  });
};


function fromConfig({ pcapPath = '.', config, }) {
  bindings.setPcapPath(pcapPath);
  return bindings.runFromConfig(validate(autoGen({ ...config, runId: uuidv4() })), { pcapPath });
}


class Network {

  constructor(options = {}) {
    this.id = uuidv4();
    this.config = {
      nodes: [],
      edges: [],
      options,
    };
  }

  setOptions(options = {}) {
    this.config.options = { ...this.config.options, ...options };
    return this.options;
  }

  isValidNode(node) {
    return (node instanceof Node && node.parentNetworkId == this.id);
  }

  addNode(node) {
    if (node instanceof Node) {
      node.parentNetwork = this;
      node.parentNetworkId = this.id;
      //this.config.nodes.push(node.toObject());
      this.config.nodes.push(node);
      return node;
    }
    throw Error('Network.addNode accepts Node object');
  }

  removeNode(node) {
    if (node instanceof Node) {
      node.parentNetwork = null;
      node.parentNetworkId = null;
      const idx = this.config.nodes.findIndex(e => e.id == node.id);
      if (idx != -1) {
        this.config.nodes.splice(idx, 1);
      }
      return node;
    }
    throw Error('Network.addNode accepts Node object');
  }

  connectNodes(nodeA, nodeB, options = {}) {
    if (!this.isValidNode(nodeA) || !this.isValidNode(nodeB)) {
      throw Error('Network.connectNodes accepts Node object');
    }
    const con = { source: nodeA.id,
      target: nodeB.id,
      type: 'default',
      ...options,
    };
    this.config.edges.push(con);
    return con;
  }

  disconnectNodes(nodeA, nodeB) {
    if (!this.isValidNode(nodeA) || !this.isValidNode(nodeB)) {
      throw Error('Network.disconnectNodes accepts Node object');
    }
    const { id: idA } = nodeA;
    const { id: idB } = nodeB;
    const conIdx = this.config.edges.findIndex(e => {
      const [source, target] = [e.source, e.target];
      return (source == idA && target == idB) || (source == idB && target == idA);
    });
    if (conIdx != -1) {
      this.config.edges.splice(conIdx, 1);
    }
  }

  toObject() {
    const config = { ...this.config, nodes: [...this.config.nodes], edges: [...this.config.edges] };
    for (let i = 0; i < config.nodes.length; ++i) {
      config.nodes[i] = this.config.nodes[i].toObject();
      const { id } = config.nodes[i];
      config.nodes[i].id = i;
      for (const con of config.edges) {
        if (con.source == id) {
          con.source = i;
        }
        if (con.target == id) {
          con.target = i;
        }
      }
    }
    return config;
  }

  run(pcapPath, { upload = false, createImage = false, ...other } = {}) {
    if (!fs.existsSync(pcapPath)) {
      fs.mkdirSync(pcapPath, { recursive: true });
    }
    const config = this.toObject();
    config.runId = uuidv4();
    config.options = { ...config.options, ...other };
    if (!upload && !createImage) {
      return fromConfig({ pcapPath, config });
    }
    const files = fromConfig({ pcapPath, config });
    const configPath = path.resolve(pcapPath, 'config.json');
    fs.writeFileSync(configPath, JSON.stringify(config));
    files.push(configPath);
    if (upload) {
      return new Promise((resolve, reject) => {
        import('got').then(({ default: got }) => {
          try {
            const req = got.stream.post(url);
            req.on('response', (resp) => {
              streamToString(resp).then(str => {
                const data = JSON.parse(str);
                if (data.message == 'ok') {
                  resolve(data.data);
                }
                else {
                  reject(data);
                }
              });
            });
            pipeline(
              tar.pack(pcapPath, { entries: files.map(e => path.basename(e)) }),
              createGzip(),
              req,
              new stream.PassThrough(),
            ).catch(err => {throw Error('Upload error (the upload limit is 50Mb)')});
          } catch(e) {
            throw Error('Upload error (the upload limit is 50Mb)');
          }
        });
      });
    }
    if (createImage) {
      const tmpFn = path.join(os.tmpdir(), `${uuidv4()}.tgz`);
      return pipeline(
        tar.pack(pcapPath, { entries: files.map(e => path.basename(e)) }),
        createGzip(),
        fs.createWriteStream(tmpFn),
      ).then(() => (fs.copyFileSync(tmpFn, path.join(pcapPath, 'image.tgz')), fs.unlinkSync(tmpFn), path.resolve(pcapPath, 'image.tgz')));
    }
  }
}

module.exports = { fromConfig, Network, ...Nodes, ...Applications };
