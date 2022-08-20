var Boxes = require("../../Boxes/Boxes"),
    BoxesLayout = require("../../Layouts/BoxesLayout");

function Traces() {

  var self = this;
  
  $(document).on("click", "[data-tracepoint-name][data-add-trace]", function () {
    self.addTracePoint($(this).attr("data-tracepoint-name"));
  });

  $(document).on("click", "[data-tracepoint-name][data-remove-trace]", function () {
    self.removeTracePoint($(this).attr("data-tracepoint-name"));
  });

  $(document).on("click", ".clearAll", function () {
    self.clearAll();
  });

  $(document).on("keyup", "#filterTracepoints", function () {
    self.filterTracepoints($(this).val());
  });
}

Traces.prototype.clearAll = function() {
  var self = this;
  this.tracePointsBox.removeAll().then(function () {
    self.traceListBox.removeAll();
  });
};

Traces.prototype.filterTracepoints = function(keyword) {
  this.traceListBox.filter(keyword);
};

Traces.prototype.refresh = function() {
  this.tracePointsBox.refresh();
};

Traces.prototype.addTracePoint = function(id) {
  this.tracePointsBox.add(id);
  this.traceListBox.add(id);
};

Traces.prototype.removeTracePoint = function(id) {
  this.tracePointsBox.remove(id);
  this.traceListBox.remove(id);
};

Traces.prototype.setContainer = function() {
  this.getLayoutContainer().append(
    '<div class="row" style="height: inherit;">' +
              '<div id="profiler" class="roundedContainer col-lg-12">' +
              '</div>' +
    '</div>'
  );
  this.layoutContainerID = "profiler";
};

Traces.prototype.handler = function() {
  this.tracePointsBox = new Boxes.TracePoints;
  this.traceListBox = new Boxes.TraceList
  this.layout = new BoxesLayout([
    this.tracePointsBox, this.traceListBox
  ]);
  this.layout.preRender = this.setContainer.bind(this.layout);
  this.layout.render();
};

module.exports = Traces;
