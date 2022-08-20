var Jolokia = require("../Jolokia"),
  CassandraGraph = require("./CassandraGraph"),
  Cassandra = require("./Cassandra"),
  helpers = require("../../helpers"),
  apiGETCall = helpers.apiGETCall;


function CassandraOperationsGraph() {
  var self = this;
  Cassandra.ifIsRunning().then(function (isRunning) {
    if (isRunning) self.startPulling();
  });
}

CassandraOperationsGraph.prototype = Object.create(CassandraGraph.prototype);

CassandraOperationsGraph.prototype.reads = new helpers.DerivativePlot();
CassandraOperationsGraph.prototype.mutations = new helpers.DerivativePlot();
CassandraOperationsGraph.prototype.gossip = new helpers.DerivativePlot();

CassandraOperationsGraph.prototype.pullData = function () {
  var self = this;
  $.when(
    Jolokia.read("org.apache.cassandra.request:type=ReadStage"),
    Jolokia.read("org.apache.cassandra.request:type=MutationStage"),
    Jolokia.read("org.apache.cassandra.internal:type=GossipStage")
  ).then(function (read, mutation, gossip) {

    self.reads.add(read.timestamp, read.value.ActiveCount);
    self.mutations.add(mutation.timestamp, mutation.value.ActiveCount);
    self.gossip.add(gossip.timestamp, gossip.value.ActiveCount);

  })
};


CassandraOperationsGraph.prototype.getData = function () {
  return [
    this.safePlot(this.reads.getPlot()),
    this.safePlot(this.mutations.getPlot()),
    this.safePlot(this.gossip.getPlot())
  ]
};

var singleton = new CassandraOperationsGraph();

module.exports = singleton;
