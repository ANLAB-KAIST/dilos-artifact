var Settings = require("../Settings"),
    StaticBox = require("./StaticBox"),
    Trace = require("../API/Trace");

function TraceList() {
}


TraceList.prototype = new StaticBox();

TraceList.prototype.template = "/osv/templates/boxes/TraceList.html";

TraceList.prototype.filter = function(keyword) {
  $('.rightBoxCheckElements [data-tracepoint-name]').each(function () {
    var $trace = $(this);
    if ($trace.html().indexOf(keyword) == -1) {
      $trace.hide();
    } else {
      $trace.show();
    }
  })
};

TraceList.prototype.add = function(id) {
  if ($(".selectedTracepoints [data-tracepoint-name='"+id+"']").length) {
    return;
  }
  var selectedTmpl = '<div class="checkbox" data-remove-trace data-tracepoint-name="' + id + '">' +
              '<label>' +
                  '<div class="unselect"></div><span class="selectedThreads">' + id + '</span>' +
              '</label>' + 
          '</div>' +
          '<hr style="margin:0px"/>';
  $(".traceList [data-tracepoint-name='"+id+"'], .traceList [data-tracepoint-name='"+id+"'] + hr").remove();
  $(".selectedTracepoints").append(selectedTmpl);

};

TraceList.prototype.remove = function(id) {
  var $trace = $(".selectedTracepoints [data-tracepoint-name='"+id+"']");
  $trace.next().remove();
  $trace.remove();
  var unselectedTmpl = '<div class="checkbox" data-add-trace data-tracepoint-name="' + id + '">' +
      '<label>' +
          '<div class="select"></div><span class="unselectedThreads">' + id + '</span>' +
      '</label>' + 
  '</div>' +
  '<hr style="margin:0px"/>';

  $(".traceList").append(unselectedTmpl);
  this.refreshListFromAPI();
};

TraceList.prototype.removeAll = function () {
  var selected = $(".selectedTracepoints")
  .find("[data-tracepoint-name]")
  .map(function () {
    return $(this).attr("data-tracepoint-name");
  })
  .toArray()
  .map(this.remove.bind(this));
};

TraceList.prototype.refreshListFromAPI = function () {
  var self = this;
  Trace.counts().then(function (a) {
    if (!a.list) return;
    $.map(a.list, function (data, id) {
      self.add(id);
    });
  });
};

TraceList.prototype.postRender = function () {
  this.refreshListFromAPI();      
};

TraceList.prototype.fetchData = function () {
  return Trace.all(function (args) {
    return args;
  })
};

module.exports = TraceList;
