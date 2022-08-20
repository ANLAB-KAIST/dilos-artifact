var Settings = require("../Settings"),
    StaticBox = require("./StaticBox"),
    OS = require("../API/OS"),
    helpers = require("../helpers");

function StaticInfo() {
  this.interval = setInterval(this.refresh.bind(this), Settings.DataFetchingRate)
}

StaticInfo.prototype = new StaticBox();

StaticInfo.prototype.refresh = function () {
  var container = $(this.selector);
  this.fetchData().then(function (data) {
    data.forEach(function (obj) {
      container.find("[data-key='"+obj.key+"']").html(obj.value);
    })
  });
};

StaticInfo.prototype.clear = function() {
  clearInterval(this.interval);
  $(this.selector).remove();
};

StaticInfo.prototype.getData = function() {
  return $.when(
    OS.getHostname(),
    OS.Memory.total(),
    OS.Memory.free(),
    OS.uptime(),
    OS.version()
  );
};

StaticInfo.prototype.formatUptime = function(ms) {
  var x, seconds, minutes, hours, days, uptime = "";
  x = ms / 1000;
  seconds = Math.floor(x % 60);
  x /= 60;
  minutes = Math.floor(x % 60);
  x /= 60;
  hours = Math.floor(x % 24);
  x /= 24;
  days = Math.floor(x);

  uptime +=  days + " Days, ";
  uptime +=  hours + " Hours, ";
  uptime +=  minutes + " Minutes, ";
  uptime +=  seconds + " Seconds. ";
  
  return uptime;

}
StaticInfo.prototype.parseData = function(hostname, totalMemory, freeMemory, uptime, version) {
  return [
    { key: "Host name", value: hostname },
    { key: "Memory Total", value: helpers.humanReadableByteSize(totalMemory) },
    { key: "Memory Free", value: helpers.humanReadableByteSize(freeMemory) },
    { key: "Uptime", value: this.formatUptime(uptime * 1000) },
    { key: "OSv version", value: version }
  ];
};

StaticInfo.prototype.fetchData = function() {
  return this.getData().then(this.parseData.bind(this));
};

module.exports = StaticInfo;
