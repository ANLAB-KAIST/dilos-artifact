var Jolokia = require("../Jolokia"),
  Cassandra = require("./Cassandra"),
  CassandraGraph = require("./CassandraGraph"),
  Settings = require("../../Settings"),
  apiGETCall = require("../../helpers").apiGETCall;

function CassandraLatencyGraph() {
  var self = this;
  Cassandra.ifIsRunning().then(function (isRunning) {
    if (isRunning) self.startPulling();
  });
}
CassandraLatencyGraph.prototype = Object.create(CassandraGraph.prototype);

CassandraLatencyGraph.prototype.range = [];

CassandraLatencyGraph.prototype.read = [];

CassandraLatencyGraph.prototype.write = [];

CassandraLatencyGraph.prototype.pullData = function () {
  var self = this;
  $.when(
    Jolokia.read("org.apache.cassandra.db:type=StorageProxy/RecentRangeLatencyMicros"),
    Jolokia.read("org.apache.cassandra.db:type=StorageProxy/RecentReadLatencyMicros"),
    Jolokia.read("org.apache.cassandra.db:type=StorageProxy/RecentWriteLatencyMicros")
  ).then(function (range, read, write) {
    self.range.push([range.timestamp, range.value])
    self.read.push([read.timestamp, read.value])
    self.write.push([write.timestamp, write.value])
  })
};

CassandraLatencyGraph.prototype.getData = function() {
  return [
    this.safePlot(this.range),
    this.safePlot(this.read),
    this.safePlot(this.write)
  ]
}
CassandraLatencyGraph.prototype.startPulling = function () {
  this.interval = setInterval(this.pullData.bind(this), Settings.DataFetchingRate);
};

var singleton = new CassandraLatencyGraph();

module.exports = singleton;
