var Jolokia = require("../Jolokia"),
  CassandraGraph = require("./CassandraGraph"),
  Cassandra = require("./Cassandra"),
  helpers = require("../../helpers"),
  apiGETCall = helpers.apiGETCall;


function CassandraDBGraph() {
  var self = this;
  Cassandra.ifIsRunning().then(function (isRunning) {
    if (isRunning) self.startPulling();
  });
}

CassandraDBGraph.prototype = Object.create(CassandraGraph.prototype);

CassandraDBGraph.prototype.completedTasks = new helpers.DerivativePlot();

CassandraDBGraph.prototype.pullData = function () {
  var self = this;
  $.when(
    Jolokia.read("org.apache.cassandra.db:type=Commitlog")
  ).then(function (Commitlog) {
    self.completedTasks.add(Commitlog.timestamp, Commitlog.value.CompletedTasks);
  });
};

CassandraDBGraph.prototype.getData = function() {
  return [
    this.safePlot(this.completedTasks.getPlot())
  ]
};

var singleton = new CassandraDBGraph();

module.exports = singleton;
