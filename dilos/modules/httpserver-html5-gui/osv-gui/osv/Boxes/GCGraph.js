var GraphBox = require("./GraphBox"),
    JVM = require("../API/JVM");
    
function GCGraph() {
  GraphBox.call(this, arguments)
}

GCGraph.prototype = Object.create(GraphBox.prototype);

GCGraph.prototype.title = "Garbage Collection";

GCGraph.prototype.extraSettings = function() {
  return {
    axes: {
      xaxis: {
        renderer: $.jqplot.DateAxisRenderer,
        tickOptions: {
          formatString: "%H:%M:%S"
        },
        label: "Time"
      },
      yaxis: {
        min: 0,
        max: this.total
      }
    },
    series: JVM.GCGraphAPI.labels.map(function (label) {
      return {
        label: label,
      }
    }),
  }
};

GCGraph.prototype.fetchData = function() {
  return $.Deferred().resolve(JVM.GCGraphAPI.getData());
};

module.exports = GCGraph;
