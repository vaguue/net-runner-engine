const ipRex = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/;

const dataRateSuffixes = ['bps', 'b/s', 'Bps', 'B/s', 'kbps', 'kb/s', 'Kbps', 'Kb/s', 'kBps', 'kB/s', 'KBps', 'KB/s', 'Kib/s', 'KiB/s', 'Mbps', 'Mb/s', 'MBps', 'MB/s', 'Mib/s', 'MiB/s', 'Gbps', 'Gb/s', 'GBps', 'GB/s', 'Gib/s', 'GiB/s'];

const timeSuffixes = ['s', 'ms', 'us', 'ns', 'ps', 'fs'];

const is = {
  object: e => typeof e == 'object',
  number: e => !Number.isNaN(Number(e)),
  array: e => Array.isArray(e),
  ip: e => ipRex.test(e),
  dataRate: (e) => {
    if (typeof e != 'string') return false;
    const val = parseFloat(e);
    if (Number.isNaN(val)) return false;
    const rest = e.slice(val.toString().length);
    if (!dataRateSuffixes.includes(rest)) return false;
    return true;
  },
  time: (e) => {
    if (typeof e != 'string') return false;
    const val = parseFloat(e);
    if (Number.isNaN(val)) return false;
    const rest = e.slice(val.toString().length);
    if (!timeSuffixes.includes(rest)) return false;
    return true;
  }
};

module.exports = is;
