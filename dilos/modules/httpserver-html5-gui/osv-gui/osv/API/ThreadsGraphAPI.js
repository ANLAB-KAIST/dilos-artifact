var GraphAPI = require("./GraphAPI"),
    helpers = require("../helpers");

function ThreadsGraphAPI() {
  this.path = "/os/threads";
  this.data = [];
  this.startPulling();
};

ThreadsGraphAPI.prototype = Object.create(GraphAPI.prototype);

ThreadsGraphAPI.prototype.prevTime = {};

ThreadsGraphAPI.prototype.timems = 0;

ThreadsGraphAPI.prototype.idles = {};

ThreadsGraphAPI.prototype.threads = {};

ThreadsGraphAPI.prototype.names = {};

ThreadsGraphAPI.prototype.statuses = {};

ThreadsGraphAPI.prototype.formatResponse = function (threads) { 

  var timestamp = threads.time_ms,
    self = this,
    parsedThreads,
    newTimems = threads.time_ms,
    diff = {},
    idles = {};

  threads.list.forEach(function (thread) {
    
    self.names[ thread.id ] = thread.name; 
    self.statuses[ thread.id ] = thread.status;
    if (self.timems) {
      diff[ thread.id ] = thread.cpu_ms - self.prevTime[ thread.id ];
    }
    
    self.prevTime[ thread.id ] = thread.cpu_ms;  

    if ( thread.name.indexOf("idle") != -1 && self.timems) {
      idles[ thread.id ] = {
        diff: diff[ thread.id ],
        id: thread.id,
        name: thread.name,
        cpu_ms: thread.cpu_ms
      };
    }

  });

  $.map(idles, function (idle) {
    
    self.idles[ idle.id ] = self.idles[ idle.id ] || idle;

    var percent = Math.max(0,(100 - (100 * idle.diff) / (newTimems - self.timems)));

    self.idles[ idle.id ].cpu_ms = idle.cpu_ms; 
    
    self.idles[ idle.id ].plot = self.idles[ idle.id ].plot || [];

    self.idles[ idle.id ].plot.push([ timestamp,  percent]);

  })

  $.map(diff, function (val, key) {
    return [[key, val]]
  }).sort(function (t1, t2) {
    return t1[1] > t2[1] ? -1 : 1;
  }).forEach(function (diff) {
    var id = diff[0]|0;
    
    if (self.names[ id ].indexOf("idle") != -1) return;

    var percent = (100 * diff[1]) / (newTimems - self.timems);
    if (!self.threads[ id ]) {
      self.threads[ id ] = {id : id, name: self.names[ id ], plot: [], statusTimeline: [] }
    }

    self.threads[ id ].status = self.statuses[ id ];
    
    self.threads[ id ].statusTimeline.push({
      time: timestamp,
      status: self.statuses[ id ]
    });

    self.threads[ id ].plot.push([
      timestamp,
      percent
    ]);

  });

  self.timems = newTimems;

};

ThreadsGraphAPI.prototype.getThreads = function() {
  return this.threads;
};

ThreadsGraphAPI.prototype.averageCpus = function(cpus) {
  cpus = $.map(cpus, function (cpu) { return cpu; });
  var timestamp, pointsToAverage;

  return cpus[0].plot.map(function (point, idx) {
    timestamp = point[0];
    pointsToAverage = cpus.map(function (cpu) {
      return cpu.plot[idx][1];
    });
    return [ timestamp, helpers.averageArray(pointsToAverage) ];
  });
};

ThreadsGraphAPI.prototype.hasCPUData = function() {
  return Object.keys(this.threads).length > 1;
}
ThreadsGraphAPI.prototype.getCpuAverage = function() {
  return this.hasCPUData() ? this.averageCpus(this.idles) : [];
};

ThreadsGraphAPI.prototype.getData = function () {
  return $.Deferred().resolve(this.getThreads());
};

ThreadsGraphAPI.prototype.getCpu = function () {
  return this.idles;
};

module.exports = ThreadsGraphAPI;
