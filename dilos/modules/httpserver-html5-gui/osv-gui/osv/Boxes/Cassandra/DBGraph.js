var GraphBox = require("../GraphBox"),
    CassandraDBGraph = require("../../API/Applications/CassandraDBGraph");

function DBGraph() {
  GraphBox.call(this, arguments);
}

DBGraph.prototype = Object.create(GraphBox.prototype);

DBGraph.prototype.title = "DB";

DBGraph.prototype.extraSettings = function() {
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
        label: "Completed Tasks",
      }
    ],
  }
};

DBGraph.prototype.fetchData = function() {
  var data = CassandraDBGraph.getData();
  return $.Deferred().resolve(data);
};

module.exports = DBGraph;
