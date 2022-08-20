var SideTextGraphBox = require("../SideTextGraphBox"),
    CassandraOperationsGraph = require("../../API/Applications/CassandraOperationsGraph");
    
function OperationsGraph() {
  SideTextGraphBox.call(this, arguments)
}

OperationsGraph.prototype = Object.create(SideTextGraphBox.prototype);

OperationsGraph.prototype.title = "Completed Tasks";

OperationsGraph.prototype.extraSettings = function() {
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
        label: "Read"
      },

      {
        label: "Mutation"
      },

      {
        label: "Gossip"
      }

    ],
  }
};
OperationsGraph.prototype.getSideText = function () {
  return [
    {
      label: "Read - Active",
      value: CassandraOperationsGraph.readsActiveCount,
      unit: ""
    },

    {
      label: "Mutation - Active",
      value: CassandraOperationsGraph.mutationsActiveCount,
      unit: ""
    },

    {
      label: "Gossip - Active",
      value: CassandraOperationsGraph.gossipActiveCount,
      unit: ""
    },

  ];
};

OperationsGraph.prototype.fetchData = function() {
  var data = CassandraOperationsGraph.getData();
  return $.Deferred().resolve(data);
};


module.exports = OperationsGraph;
