var Settings = require("../Settings"),
    StaticBox = require("./StaticBox"),
    Trace = require("../API/Trace");

function TracePoints() {
  this.run();
  $(document).on("play", this.run.bind(this));
  $(document).on("pause", this.pause.bind(this));
}

TracePoints.prototype = new StaticBox();

TracePoints.prototype.template = "/osv/templates/boxes/TracePoints.html";

TracePoints.prototype.charts = {};
TracePoints.prototype.lines = {};

TracePoints.prototype.addGraph = function(point) {
  var smoothie = new SmoothieChart({minValue:0, millisPerPixel:100, grid:{fillStyle:'#ffffff'}, labels: {fillStyle:'#000000'}}),
    line =  new TimeSeries();

  this.charts[point.name] = smoothie;
  this.lines[point.name] = line;
  smoothie.addTimeSeries(line,{ strokeStyle: 'rgba(0, 255, 0, 1)', fillStyle: 'rgba(0, 255, 0, 0.2)', lineWidth: 4 });
  smoothie.streamTo(document.getElementById("smoothie-"+point.name));

}
TracePoints.prototype.addTracepoint = function(point) {
  var template = $("[data-template-path='/osv/templates/boxes/TracePointLine.html']").html();
  var view = Handlebars.compile(template);
  var html = view(point);
  $(this.selector).find("tbody").append(html);
  this.addGraph(point);
};

TracePoints.prototype.update = function(tracepoints) {
  var self = this;
  if (typeof tracepoints == "undefined") return;
  $.map(tracepoints, function (point) {
    var $point = $("[data-tracepoint='"+point.name+"']"),
      rendered = $point.length > 0,
      $count = $point.find(".count");
    if (rendered) {
      $count.html(point.change);
      self.lines[point.name].append(new Date().getTime(), point.change)
    } else {
      self.addTracepoint(point);
    }
  })
};

TracePoints.prototype.refresh = function() {
  this.fetchData().then(function (res) {
    return res.list;
  }).then(this.update.bind(this));
};

TracePoints.prototype.add = function(id) {
  Trace.addTrace(id);
  this.refresh();
};

TracePoints.prototype.remove = function(id) {
  Trace.deleteTrace(id);
  $("[data-tracepoint='"+id+"']").remove();
};

TracePoints.prototype.removeAll = function(id) {
  $("[data-tracepoint]").remove();
  return Trace.deleteAll();
};

TracePoints.prototype.postRender = function() {
  this.refresh();
};

TracePoints.prototype.fetchData = function () {
  return Trace.counts()
};

TracePoints.prototype.pause = function () {
  $.map(this.charts, function (chart) {
    chart.stop();
  });
  clearInterval(this.interval);
};

TracePoints.prototype.run = function () {
  this.interval = setInterval(this.refresh.bind(this), Settings.DataFetchingRate)
  $.map(this.charts, function (chart) {
    chart.start();
  });
};

TracePoints.prototype.clear = function() {
  clearInterval(this.interval);
  $(this.selector).remove();
};

module.exports = TracePoints;
