var BoxesLayout = require("../../Layouts/BoxesLayout"),
    StaticInfo = require("../../Boxes/Cassandra/StaticInfo"),
    LatencyGraph = require("../../Boxes/Cassandra/LatencyGraph"),
    DBGraph = require("../../Boxes/Cassandra/DBGraph"),
    OperationsGraph = require("../../Boxes/Cassandra/OperationsGraph"),
    CompactionGraph = require("../../Boxes/Cassandra/CompactionGraph");

function Cassandra() {
}

Cassandra.prototype.handler = function() {
  this.layout = new BoxesLayout([
    new StaticInfo(), new LatencyGraph(),
    new OperationsGraph(), new DBGraph(),
    new CompactionGraph(),
  ]);
  this.layout.render();
};

module.exports = Cassandra;
