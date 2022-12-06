const files = ['onTick', 'ping', 'rawConfig', 'readme', 'routing', 'switch', 'switchPing'];

async function main() {
  for (const file of files) {
    console.log('[*]', file);
    const main = require(`./${file}`);
    await main();
  }
  process.exit(0);
}

main().catch(console.error);
