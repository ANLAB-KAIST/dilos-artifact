var apiGETCall = require("../helpers").apiGETCall,
  apiPOSTCall = require("../helpers").apiPOSTCall,
  GraphAPI = require("./GraphAPI"),
  ThreadsGraphAPI = require("./ThreadsGraphAPI"),
  freeMemoryGraph = new GraphAPI("/os/memory/free"),
  totalMemoryGraph = new GraphAPI("/os/memory/total"),
  threadsGraph;

threadsGraph = new ThreadsGraphAPI(); 

module.exports = { 
  version: apiGETCall("/os/version"),
  manufactor: apiGETCall("/os/manufactor"),
  uptime: apiGETCall("/os/uptime"),
  date: apiGETCall("/os/date"),
  Memory: {
    total: apiGETCall("/os/memory/total"),
    free: apiGETCall("/os/memory/free"),
    History: {
      total: totalMemoryGraph.getData.bind(totalMemoryGraph),
      free: freeMemoryGraph.getData.bind(freeMemoryGraph)
    }
  },
  shutdown: apiPOSTCall("/os/shutdown"),
  dmesg: apiGETCall("/os/dmesg"),
  getHostname: apiGETCall("/os/hostname"),
  setHostname: apiPOSTCall("/os/hostname"),
  threads: apiGETCall("/os/threads"),
  threadsGraph: threadsGraph.getData.bind(threadsGraph),
  CPUAverage: threadsGraph.getCpuAverage.bind(threadsGraph),
  CPU: threadsGraph.getCpu.bind(threadsGraph)
};
