var StaticBox = require("./StaticBox"),
    OS = require("../API/OS");

function ThreadsTableBox() {
  var self = this;
  $(document).on("keyup", ".filterThreads", function (event) {
    var value = $(event.target).val();
    self.filterThreads(value)
  });
}


ThreadsTableBox.prototype = new StaticBox();
ThreadsTableBox.prototype.renderTo = "#profiler";
ThreadsTableBox.prototype.template = "/osv/templates/boxes/ThreadsTable.html";

ThreadsTableBox.prototype.drawUpdates = function (thread) {
  var $thread = $(".threadsList [data-thread-id='"+thread.id+"']");
  $thread.find(".cpuUsage").html(thread.percent)
  $thread.next(".bar").css("width", thread.percent+"%")
};

ThreadsTableBox.prototype.draw = function (thread) {
  if ($("[data-thread-id='"+thread.id+"']").length) {
    this.drawUpdates(thread);
  } else {
    var source = $("[data-template-path='/osv/templates/boxes/ThreadListItem.html']").html(),
      template = Handlebars.compile(source),
      html;
    thread.selected = this.selected.indexOf(parseInt(thread.id, 0)) != -1;
    html = template(thread);
    if (thread.selected) {
      $
      $(".selectedThreads").append(html);
    } else {
      $(".unselectedThreads").append(html);
    }
  }
};

ThreadsTableBox.prototype.refresh = function (selected) {
  var self = this;
  this.selected = selected;

  this.fetchData().then(function (res) {
    res.threads.forEach(self.draw.bind(self));
  });
};

ThreadsTableBox.prototype.selected = [];

ThreadsTableBox.prototype.moveThread = function(id, newParentSelector) {
  var $thread = $(".threadsList [data-thread-id='"+id+"']").parents(".thread");
  $thread.find(".toggleThread").toggleClass("checked")
  $thread.next("hr").remove();
  $thread.detach();
  $thread.appendTo(newParentSelector);
  if ($thread.find(".select").length) {
    $thread.find(".select").addClass("unselect").removeClass("select");
  } else {
    $thread.find(".unselect").addClass("select").removeClass("unselect");
  }
  $(".threadsList [data-thread-id='"+id+"']").parents(".thread").after("<hr style='margin-top:0px'>");

};

ThreadsTableBox.prototype.select = function (id) {
  id = parseInt(id,10);
  if (this.selected.indexOf(id) == -1) this.selected.push();
  this.moveThread(id, ".selectedThreads");
};

ThreadsTableBox.prototype.unselect = function (id) {
  id = parseInt(id,10);
  var idx = this.selected.indexOf(id);
  this.selected.splice(idx);
  this.moveThread(id, ".unselectedThreads");
};

ThreadsTableBox.prototype.filterThreads = function (val) {
  if (!val) val = "";
  $(".threadsList .thread").each(function () {
    var $el = $(this);
    if ($el.attr("data-thread-name").indexOf(val) == -1) {
      $el.hide();
      $el.next().hide();
    } else {
      $el.show();
      $el.next().show()
    }
  })
};

ThreadsTableBox.prototype.fetchData = function () {
  return OS.threadsGraph().then(function (threads) {
    
    var threads = $.map(threads, function (thread, id) {
      return {
        name: thread.name,
        id: id,
        percent: thread.plot[thread.plot.length - 1][1].toFixed(1)
      }
    });
    
    return { 
        threads: threads, 
        selected: threads.map(function (thread) { return thread.id}).slice(0,9) 
      }
  })
};

module.exports = ThreadsTableBox;
