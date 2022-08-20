var Settings = require("../../Settings");

function CassandraGraph() {

}

CassandraGraph.prototype.startPulling = function () {
  this.interval = setInterval(this.pullData.bind(this), Settings.DataFetchingRate);
};

CassandraGraph.prototype.normalizePlotTimestamps = function (point) {
  if (point.length != 2) return point;
  return [point[0] < Math.pow(10,10)? point[0] * 1000 : point[0], point[1]]
};

CassandraGraph.prototype.safePlot = function (plot) {
  return (plot.length > 0 ? plot : [[]]).slice(-Settings.Graph.MaxTicks).map(this.normalizePlotTimestamps);
};

module.exports = CassandraGraph;
