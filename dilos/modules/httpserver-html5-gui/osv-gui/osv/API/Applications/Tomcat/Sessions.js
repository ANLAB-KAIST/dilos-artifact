var Jolokia = require("../../Jolokia"),
  CassandraGraph = require("../CassandraGraph"),
  Tomcat = require("./Tomcat"),
  Settings = require("../../../Settings"),
  apiGETCall = require("../../../helpers").apiGETCall

function Sessions() {
  var self = this;
  Tomcat.ifIsRunning().then(function (isRunning) {
    if (isRunning) self.startPulling();
  });
}

Sessions.prototype = Object.create(CassandraGraph.prototype);

Sessions.prototype.plots = null;

Sessions.prototype.pullData = function () {
  var self = this;
  Jolokia.read("Catalina:context=*,host=*,type=Manager/activeSessions?ignoreErrors=true")
  .then(function (res) {
    if (self.plots == null) self.plots = {};
    var timestamp = res.timestamp * 1000;
    $.map(res.value, function (value, key) {
      var context = key.match(/context=(.*?)($|,)/)[1],
        activeSessions = value.activeSessions;
      self.plots[context] = self.plots[context] || [];
      self.plots[context].push([ timestamp, activeSessions])
    });
  })
};

Sessions.prototype.getPlots = function () {
  if (this.plots == null) return [[null]];
  return $.map(this.plots, function (plot) {
    return [plot.slice(-Settings.Graph.MaxTicks)];
  });
};

Sessions.prototype.getLabels = function () {
  if (this.plots == null) return [];
  return $.map(this.plots, function (plot, name) {
    return name;
  });
};

var singleton = new Sessions();

module.exports = singleton;
