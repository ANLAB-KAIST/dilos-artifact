var GraphBox = require("./GraphBox"),
    JVM = require("../API/JVM"),
    helpers = require("../helpers");

function MemoryPoolGraph() {
  GraphBox.call(this, arguments)
}

MemoryPoolGraph.prototype = Object.create(GraphBox.prototype);

MemoryPoolGraph.prototype.title = "Memory Pools";

MemoryPoolGraph.prototype.extraSettings = function() {
  return {
    axes: {
      xaxis: {
        renderer: $.jqplot.DateAxisRenderer,
        tickOptions: {
          formatString: "%H:%M:%S"
        }
      },
      yaxis: {
        max: this.total,
        min: 0,
        tickOptions: {
          formatter: function(foramt, val) {
            return helpers.humanReadableByteSize(val);
          }
        }
      }
    },
    series: JVM.MemoryPoolGraph.labels.map(function (label) {
      return {
        label: label,
      }
    }),
  }
};

MemoryPoolGraph.prototype.fetchData = function() {
  return $.Deferred().resolve(JVM.MemoryPoolGraph.getData());
};

module.exports = MemoryPoolGraph;
