/* examples config TODO require all ids to be 0 ... N ?
 * { 
 *   nodes: [{ id, title, x, y, type, applications  }], 
 *   edges: [{ source, target, type, sourceIP, targetIP  }], 
 *   options: { animeLen: 10, popuplateIP: true }  
 * }
 */

const config = {
  nodes: [
    {
      id: 0,
      x: 100,
      y: 100,
      type: 'pc',
      applications: [
        {
          type: 'tcp-client',
          addr: '192.168.1.2',
          port: 3000,
        }
      ],
    },
    {
      id: 1,
      x: 150,
      y: 150,
      type: 'hub',
    },
    {
      id: 2,
      x: 200,
      y: 200,
      type: 'pc',
      applications: [
        {
          type: 'tcp-server',
          port: 3000,
        }
      ],
    },
    {
      id: 3,
      x: 300,
      y: 400,
      type: 'pc',
      applications: [ ],
    }
  ],
  edges: [
    { source: 0, target: 1, sourceIP: '192.168.1.3', targetIP: '192.168.1.1' },
    { source: 2, target: 1, sourceIP: '192.168.1.2', targetIP: '192.168.1.1' },
    { source: 3, target: 1, sourceIP: '192.168.1.4', targetIP: '192.168.1.0' },
  ],
  options: {
    animeLen: 3,
    populateIP: false,
    verbose: false,
  },
}

module.exports = config;
