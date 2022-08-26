const fs = require('fs');

//const binPath = '../build/Release/wrapper-native';
const binPath = '../build/Debug/wrapper-native';
const { Instance } = require(binPath);
const emu = new Instance();

function emulate({ pcapPath = '.', config, }) {
  emu.setPcapPath(pcapPath);
  emu.runFromConfig(config, { pcapPath });
}

module.exports = emulate;
