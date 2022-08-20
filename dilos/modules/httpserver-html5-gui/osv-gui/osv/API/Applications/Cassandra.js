var Jolokia = require("../Jolokia"),
  apiGETCall = require("../../helpers").apiGETCall, 
  liveNodes,
  isRunning = null,
  ifIsRunning,
  loadMap;

liveNodes = function () {
  return Jolokia.read("org.apache.cassandra.db:type=StorageService/LiveNodes").then(function (res) {
    return res.value;
  });
};

loadMap = function () {
  return Jolokia.read("org.apache.cassandra.db:type=StorageService/LoadMap").then(function (res) {
    return res.value;
  });
};

ifIsRunning = function () {
  var promise = $.Deferred();

  if (isRunning !== null) {
    promise.resolve(isRunning)
  } else {
    Jolokia.read("org.apache.cassandra.db:type=StorageService/LiveNodes")
      .then(function (res) {
        if (res.status == 200) {
          isRunning = true;
        } else {
          isRunning = false;
        }
        promise.resolve(isRunning)
      })
      .fail(function () {
        isRunning = false;
        promise.resolve(isRunning)
      })
  }
  return promise;
}

module.exports = { 
  liveNodes: liveNodes,
  loadMap: loadMap,
  ifIsRunning: ifIsRunning
};
