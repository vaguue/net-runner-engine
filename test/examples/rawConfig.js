const Simulator = require('../..');
const config = require('../rawConfig');
const path = require('path');
const fs = require('fs');

const dstDir = path.resolve(__dirname, 'files/raw-config');

const { fromConfig } = Simulator;

function main() {
  if (!fs.existsSync(dstDir)) {
    fs.mkdirSync(dstDir, { recursive: true });
  }
  return fromConfig({ config, pcapPath: dstDir});
}

module.exports = main;

if (require.main === module) {
  main();
}
