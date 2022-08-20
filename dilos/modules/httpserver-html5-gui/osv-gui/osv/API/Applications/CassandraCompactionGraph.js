var Jolokia = require("../Jolokia"),
  CassandraGraph = require("./CassandraGraph"),
  Cassandra = require("./Cassandra"),
  apiGETCall = require("../../helpers").apiGETCall;


function CassandraCompactionGraph() {
  var self = this;
  Cassandra.ifIsRunning().then(function (isRunning) {
    if (isRunning) self.startPulling();
  });
}

CassandraCompactionGraph.prototype = Object.create(CassandraGraph.prototype);

CassandraCompactionGraph.prototype.bytesCompacted = [];
CassandraCompactionGraph.prototype.bytesCompactedLastRead = null;

CassandraCompactionGraph.prototype.bytesTotalInProgress = [];

CassandraCompactionGraph.prototype.pullData = function () {
  var self = this;
  $.when(
    Jolokia.read("org.apache.cassandra.metrics:name=BytesCompacted,type=Compaction"),
    Jolokia.read("org.apache.cassandra.metrics:name=TotalCompactionsCompleted,type=Compaction")
  ).then(function (bytesCompacted, bytesTotalInProgress) {
    
    if (this.bytesCompactedLastRead == null) {
      self.bytesCompacted.push([bytesCompacted.timestamp, 0])
    } else {
      self.bytesCompacted.push([
        bytesCompacted.timestamp,
        (bytesCompacted.value.Count - self.bytesCompactedLastRead[1]) / (bytesCompacted.timestamp - self.bytesCompactedLastRead[0])
      ]);
    }
    self.bytesCompactedLastRead = [bytesCompacted.timestamp, bytesCompacted.value.Count];
    self.bytesTotalInProgress.push([bytesTotalInProgress.timestamp, bytesTotalInProgress.value.Count])

  })
};

CassandraCompactionGraph.prototype.getData = function() {
  return [
    this.safePlot(this.bytesCompacted),
    this.safePlot(this.bytesTotalInProgress)
  ]
}

var singleton = new CassandraCompactionGraph();

module.exports = singleton;
