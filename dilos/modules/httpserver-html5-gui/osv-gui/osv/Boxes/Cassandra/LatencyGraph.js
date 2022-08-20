var GraphBox = require("../GraphBox"),
    CassandraLatencyGraph = require("../../API/Applications/CassandraLatencyGraph");

function LatencyGraph() {
  GraphBox.call(this, arguments)
}

LatencyGraph.prototype = Object.create(GraphBox.prototype);

LatencyGraph.prototype.title = "Latency";

LatencyGraph.prototype.extraSettings = function() {
  return {
    axes: {
      xaxis: {
        renderer: $.jqplot.DateAxisRenderer,
        tickOptions: {
          formatString: "%H:%M:%S"
        }
      },
    },
    series: [
      {
        label: "Range Latency",
      },

      {
        label: "Read Latency"
      },

      {
        label: "Write Latency"
      }

    ],
  }
};

LatencyGraph.prototype.fetchData = function() {
  var data = CassandraLatencyGraph.getData();
  return $.Deferred().resolve(data);
    
};

module.exports = LatencyGraph;
