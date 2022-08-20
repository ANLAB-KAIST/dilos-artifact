var Settings = require("../Settings"),
    StaticBox = require("./StaticBox"),
    OS = require("../API/OS");

function CPUTableBox() {
  this.interval = setInterval(this.refresh.bind(this), Settings.DataFetchingRate)
}

CPUTableBox.prototype = new StaticBox();

CPUTableBox.prototype.template = "/osv/templates/boxes/CPUTable.html";

CPUTableBox.prototype.clear = function() {
  clearInterval(this.interval);
  $(this.selector).remove();
};

CPUTableBox.prototype.refresh = function () {
  var container =$(this.selector),
    template = this.getTemplate();

  this.fetchData().then(function (ctx) {
    container.html(template(ctx))
  });
};

CPUTableBox.prototype.fetchData = function () {
  var cpus = OS.CPU();
  if (Object.keys(cpus).length == 0) {
    return $.Deferred().resolve({
      timePoints: [],
      cpus: []
    });
  }
  var parsed = $.map(cpus, function (cpu) {
    cpu.name = cpu.name.replace("idle", "")
    cpu.usage = cpu.plot[ cpu.plot.length - 1 ][1].toFixed(2) + "%";
    cpu.running = cpu.cpu_ms;
    cpu.timeline = cpu.plot.slice(-5).map(function (point) { return "" })
    return cpu;
  });
  var timePoints = parsed[0].plot.map(function (point) {
    var date = new Date(point[0]);
    return date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds()
  }).slice(-5);
  return $.Deferred().resolve({
    timePoints: timePoints,
    cpus: parsed 
  })
};

module.exports = CPUTableBox;
