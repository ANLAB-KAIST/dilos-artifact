var Settings = require("../Settings"),
    helpers = require("../helpers");

function GraphAPI (path, formatter, getData) {
  this.path = path;
  this.data = [];
  this.startPulling();
  if (formatter) {
    this.formatResponse = formatter;
  } 
  if (getData) {
    this.getData = getData.bind(this);
  }
};

GraphAPI.prototype.formatResponse = function (response) {
  return [ response.generatedTimestamp? response.generatedTimestamp : Date.now(), response / Math.pow(1024, 2) ];
};

GraphAPI.prototype.rate = Settings.DataFetchingRate;

GraphAPI.prototype.fetchData = function() {
  var self = this,
    path = this.path;

  helpers.apiGETCall(path)()
   .then(function (response) {
     return typeof response == "string"? JSON.parse(response) : response;
   })
   .then(this.formatResponse.bind(this)).then(function (res) {
     if (res) self.data.push(res)
  });
};

GraphAPI.prototype.getData = function() {
  return this.data.slice( -1 * Settings.Graph.MaxTicks);
};

GraphAPI.prototype.startPulling = function() {
  this.fetchData();
  setInterval(this.fetchData.bind(this), this.rate);
};

module.exports = GraphAPI;
