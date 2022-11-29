const files = ['onTick', 'ping', 'rawConfig', 'readme', 'routing', 'switch'];

async function main() {
  for (const file of files) {
    console.log('[*]', file);
    const main = require(`./${file}`);
    await main();
  }
  process.exit(0);
}

main().catch(console.error);
