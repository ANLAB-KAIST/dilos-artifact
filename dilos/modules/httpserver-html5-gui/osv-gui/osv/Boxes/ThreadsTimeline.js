var BaseBox = require("./BaseBox"),
    OS = require("../API/OS");

function ThreadsTimeline() {
}

ThreadsTimeline.prototype = Object.create(BaseBox.prototype);

ThreadsTimeline.prototype.template = "/osv/templates/boxes/ThreadsTimeline.html";

ThreadsTimeline.prototype.renderTo = "#left";

ThreadsTimeline.prototype.fetchData = function (ids) {
  return OS.threadsGraph().then(function (res) {

    var data =  $.map(res, function(thread) {
      return thread;
    }).filter(function (thread) {
      if (!ids) return false;
      return ids.indexOf( thread.id ) != -1;
    }).map(function (thread) {
      thread.statusTimeline = thread.statusTimeline.slice(-10);
      return thread;
    });

    if (data.length == 0) return data;
    data.timeline = data[0].statusTimeline.map(function (status) {
      return new Date(status.time).toTimeString().split(" ")[0];
    });

    return data;

  })
};

ThreadsTimeline.prototype.refresh = function(ids) {
   var container =$(this.selector),
    template = this.getTemplate();

  this.fetchData(ids).then(function (ctx) {
    container.html(template(ctx))
  });
};

module.exports = ThreadsTimeline;
