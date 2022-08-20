var GraphBox = require("./GraphBox"),
    helpers = require("../helpers"),
    JVM = require("../API/JVM");
    
function HeapMemoryUsage() {
  GraphBox.call(this, arguments)
}

HeapMemoryUsage.prototype = Object.create(GraphBox.prototype);

HeapMemoryUsage.prototype.title = "Heap Memory Usage";

HeapMemoryUsage.prototype.extraSettings = function() {
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
            return helpers.humanReadableByteSize(val * Math.pow(1024, 2));
          }
        }
      }
    },
    series: [{
        label: "Memory",
    }],
  }
};

HeapMemoryUsage.prototype.fetchData = function() {
  var memoryUsage = JVM.HeapMemoryUsageGraph.getData();
  if (memoryUsage.length == 0) {
    memoryUsage = [ null ]
  } 
  return $.Deferred().resolve([ memoryUsage ]);
};

module.exports = HeapMemoryUsage;
