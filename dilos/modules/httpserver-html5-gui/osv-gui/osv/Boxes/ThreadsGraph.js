var Settings = require("../Settings"),
    GraphBox = require("./GraphBox"),
    OS = require("../API/OS"),
    helpers = require("../helpers");

function ThreadsGraph() {
  GraphBox.call(this, arguments);
  for (var i = 0; i < 256; i++) {
    this.colors[i] = helpers.randomColor();
  }
}

ThreadsGraph.prototype = Object.create(GraphBox.prototype);
ThreadsGraph.prototype.template = "/osv/templates/boxes/ThreadsGraph.html";
ThreadsGraph.prototype.visibleThreads = []
ThreadsGraph.prototype.threads = [];
ThreadsGraph.prototype.colors = {};
ThreadsGraph.prototype.title = "Threads";
ThreadsGraph.prototype.renderTo = "#left";

ThreadsGraph.prototype.extraSettings = function() {
  var self = this;
  return {
    axes: {
      xaxis: {
        tickOptions: {
          formatter: function (_, t) {
            // before any data comes to the graph jqplot tries rendering weird values.
            // this fixes the issue by returning an empty string in case a value below 1 shows up
            // on the x axis.
            if (t <= 1) return "";
            var now = Date.now();
            var secondsAgo = ((now - t) / 1000).toFixed(0);
            return "-" + secondsAgo + "s";
          }
        }
      }
    },
  series: this.threads.map(function (thread) {
      return {
        color: self.colors[ thread.id ],
        label: thread.id + " - " + thread.name
      }
    }),
  }
};

ThreadsGraph.prototype.normalizeData = function(data) {
  var self = this,
    plots;
  
  this.threads = $.map(data, function (thread) { 
    return thread; 
  }).filter(function (thread) {
    return self.visibleThreads.indexOf(thread.id) != -1;
  });
  
  plots = this.threads.map(function (thread) {
    return thread.plot.slice(-1 * Settings.Graph.MaxTicks);
  })

  if (plots.length === 0) {
    plots =[ [ null ] ]
  } 

  return plots;
};

ThreadsGraph.prototype.fetchData = function() {
  var self = this;
  return OS.threadsGraph().then(function(threadsData) {
    return self.normalizeData(threadsData);
  });
};

module.exports = ThreadsGraph;
