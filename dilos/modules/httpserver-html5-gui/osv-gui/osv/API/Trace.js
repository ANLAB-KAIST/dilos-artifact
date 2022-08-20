var helpers = require("../helpers"),
  apiGETCall = helpers.apiGETCall,
  apiPOSTCall = helpers.apiPOSTCall,
  apiDELETECall = helpers.apiDELETECall,
  BatchRequest = require("./BatchRequest"),
  all,
  counts,
  addTrace,
  deleteTrace,
  deleteAll,
  deleteDuplicates,
  lastCounts = {};

deleteDuplicates = function(tracepoints) {
  return tracepoints.reduce(function (acc, point) {
      if (acc[ point.name ]) return acc;
      acc[ point.name ] = point;
      return acc;
  }, {})
}

all = function() {
  return apiGETCall("/trace/status")().then(deleteDuplicates);
};


counts = function () {
  return apiGETCall("/trace/count")().then(function (res) {
    if (!res.list) return res;
    res.list = deleteDuplicates(res.list.map(function (point){
        var lastCount = lastCounts[ point.name ] || point.count ;
        point.change = point.count - lastCount;
        lastCounts[ point.name ] = point.count;
        return point;
    }));

    return res;
  });
};

addTrace = function (id) {
  return apiPOSTCall("/trace/count/"+id)({
      enabled: true
  });
};

deleteTrace = function (id) {
  return BatchRequest.ajax("POST", "/trace/count/"+id+"?enabled=false", true);
};

deleteAll = apiDELETECall("/trace/count");

module.exports = { 
  all: all, 
  counts: counts,
  addTrace: addTrace,
  deleteTrace: deleteTrace,
  deleteAll: deleteAll
};
