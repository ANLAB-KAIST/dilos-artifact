var Boxes = require("../Boxes/Boxes"),
    OS = require("../API/OS"),
    BoxesLayout = require("./BoxesLayout");

function ThreadsLayout() {
  var self = this;
  BoxesLayout.apply(this, arguments);
  this.setSelectedThreads();
  this.threadsGraph = new Boxes.ThreadsGraph();
  this.threadsTimeline = new Boxes.ThreadsTimeline();
  this.threadsTablebox = new Boxes.ThreadsTableBox();
  this.boxes = [ this.threadsGraph, this.threadsTablebox, this.threadsTimeline ]

  $(document).on("click", ".thread .toggleThread", this.onToggleThreadClick.bind(this))
}


ThreadsLayout.prototype = new BoxesLayout();

ThreadsLayout.prototype.layoutContainerID = "profiler";
ThreadsLayout.prototype.layoutContainer = function() {
  return "<div id='osvContainer'>" +
      '<div class="row" style="height: inherit;">' +
                '<div id="profiler" class="roundedContainer col-lg-12">' +
                  '<div class="col-xs-8" id="left"></div>' +
                '</div>' +
      '</div>' +
    '</div>'
};
ThreadsLayout.prototype.setSelectedThreads = function () {
  var self = this;
  return OS.threads().then(function (threads) {
    self.threadsGraph.visibleThreads = threads.list.sort(function (thread1, thread2) {
      return thread1.cpu_ms > thread2.cpu_ms ? -1 : 1;
    }).map(function (thread) { 
      return thread.id|0;
    }).slice(0, 9);
  })
}

ThreadsLayout.prototype.render = function () {
  var self = this;
  BoxesLayout.prototype.render.call(this, arguments);
  this.refreshTable();
  this.refreshTimeline();
  this.interval = setInterval(function () {
    self.refreshTable();
    self.refreshTimeline();
  }, 2000);
}
ThreadsLayout.prototype.clear = function() {
  BoxesLayout.prototype.clear.apply(this, arguments);
  clearInterval(this.interval)
};

ThreadsLayout.prototype.getSelectedThreads = function() {
  return this.threadsGraph.visibleThreads;
};

ThreadsLayout.prototype.showThread = function(threadID) {
  this.threadsGraph.visibleThreads.push(threadID|0);
  this.threadsTablebox.select(threadID);
  this.refreshGraph();
};

ThreadsLayout.prototype.hideThread = function(threadID) {
  this.threadsGraph.visibleThreads = this.threadsGraph.visibleThreads.filter(function (thread) { 
    return thread != threadID;
  });
  this.threadsTablebox.unselect(threadID);
  this.refreshGraph();
};

ThreadsLayout.prototype.refreshTimeline = function() {
  this.threadsTimeline.refresh(this.getSelectedThreads());
}
ThreadsLayout.prototype.refreshTable = function() {
  var visibleThreads = this.threadsGraph.visibleThreads;
  this.threadsTablebox.refresh(this.getSelectedThreads());
};

ThreadsLayout.prototype.refreshGraph = function() {
  this.threadsGraph.renderGraph(false, false)
};

ThreadsLayout.prototype.onToggleThreadClick = function(event) {
  var $checkbox = $(event.target).parent("[data-thread]"),
    threadID = $checkbox.attr("data-thread-id")|0;

  if ( $checkbox.is(".checked") ) {
    this.hideThread(threadID);
  } else {
    this.showThread(threadID);
  }
  this.refreshGraph();
  this.refreshTimeline();
  return false;
};

module.exports = ThreadsLayout;
