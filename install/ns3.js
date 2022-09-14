//TODO add node-gyp-build + prebuildify support
const fs = require('fs');
const path = require('path');
const os = require('os');
const detectLibc = require('detect-libc');
const tar = require('tar-fs');
const gunzip = require('gunzip-maybe');
const pipeline = require('util').promisify(require('stream').pipeline);

const { systemsSupported, sharedObjects, include } = require('./initData');

const rootDir = path.resolve(__dirname, '..');

const config = {
  enableGlobalSearch: false,
};


function dirContains(dir, files) {
  try {
    const res = files.filter(e => !fs.existsSync(path.resolve(dir, e)));
    if (res.length == 0) {
      return true;
    }
    else if (res.length < files.length) {
      console.warn(`Probably found ns-3 installation dir at ${dir}, which does not include all necessary files: `, res.join(','));
    }
    return false;
  } catch(e) {

  }
  return false;
}

const dirs = [...(process.env.PATH || '').split(':'), ...(process.env.LD_LIBRARY_PATH || '').split(':')].filter(e => e && e.length > 0);

function systemId() {
  const arch = process.env.npm_config_arch || process.arch;
  const platform = process.env.npm_config_platform || process.platform;
  const libc = process.env.npm_config_libc || (detectLibc.isNonGlibcLinuxSync() ? detectLibc.familySync() : '');
  const libcId = platform !== 'linux' || libc === detectLibc.GLIBC ? '' : libc;
  const platformId = [`${platform}${libcId}`];
  if (arch === 'arm') {
    const fallback = process.versions.electron ? '7' : '6';
    platformId.push(`armv${process.env.npm_config_arm_version || process.config.variables.arm_version || fallback}`);
  } else if (arch === 'arm64') {
    platformId.push(`arm64v${process.env.npm_config_arm_version || '8'}`);
  } else {
    platformId.push(arch);
  }
  return platformId.join('-');
}

const vendorRoot = path.resolve(__dirname, 'vendor');
const vendorLibDir = path.join(vendorRoot, 'lib');
const vendorIncludeDir = path.join(vendorRoot, 'include');
const libDir = config.enableGlobalSearch && dirs.find(e => dirContains(e, sharedObjects));
const includeDir = config.enableGlobalSearch && dirs.find(e => dirContains(e, include));

module.exports = { 
  libDir: libDir || vendorLibDir, 
  boostDir: vendorIncludeDir,
  rpath: libDir ? libDir : '$$ORIGIN/../../install/vendor/lib',
  includeDir: includeDir || vendorIncludeDir,
  libs: ['-lstdc++fs', ...sharedObjects.map(e => e.replace(/^lib/, '-l:lib'))].join(' '),
};

async function dwn(fn) {
  const got = await import('got').then(res => res.default);
  //const dwnURL = 'https://github.com/hedonist666/ns3-build/releases/download/3.29'; TODO create repo
  const dwnURL = 'https://emu.net-runner.xyz/build/';
  const url = new URL(fn, dwnURL).href;
  try {
    await pipeline(
      got.stream(url),
      gunzip(),
      tar.extract(vendorRoot),
    );
  } catch (e) {
    throw Error('[!] Download error');
  }
}

async function main() {
  try {
    const sysId = systemId();
    if (fs.existsSync(vendorRoot)) {
      fs.rmSync(vendorRoot, { recursive: true, force: true });
    }
    fs.mkdirSync(vendorRoot);
    console.log('[*] Downloading boost statis_assert (a little quirk for ns-3 openflow switch module)');
    await dwn('boost/1.67.0/static_assert.tgz');
    if (config.enableGlobalSearch && libDir && includeDir) {
      console.log('[*] Found ready ns-3 installation, building from source');
      process.exit(0);
    }
    else if (systemsSupported.includes(sysId)) {
      console.log('[*] Downloading prebuilt binaries');
      const fn = `ns-3/3.29/${sysId}.tgz`;
      await dwn(fn);
      process.exit(0);
    }
    else {
      throw Error('[*] Neither there is installed ns-3 in the system nor the system is supported');
    }
  } catch(e) {
    console.error(e);
    process.exit(1);
  }
}

if (require.main === module) {
  main().then(() => process.exit(0)).catch(console.error);
}
