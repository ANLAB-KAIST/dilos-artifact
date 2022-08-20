var StaticBox = require("../StaticBox"),
    Cassandra = require("../../API/Applications/Cassandra");


function StaticInfo() {

}

StaticInfo.prototype = new StaticBox();

StaticInfo.prototype.template = "/osv/templates/boxes/cassandra/StaticInfo.html";

StaticInfo.prototype.getData = function() {
  return $.when(
    Cassandra.liveNodes(),
    Cassandra.loadMap()
  );
};

StaticInfo.prototype.parseData = function(liveNodes, loadMap) {
  return {
    liveNodes: liveNodes,
    loadMap: loadMap
  }
};

StaticInfo.prototype.fetchData = function() {
  return this.getData().then(this.parseData.bind(this));
};

module.exports = StaticInfo;
