var helpers = require("../helpers");

function BoxesLayout(boxes) {
  this.boxes = boxes;
  this.layoutContainerID = "osvContainer";
  this.mainContainer = $("#main");
  document.addEventListener("runRoute", this.clear.bind(this))
}

BoxesLayout.prototype.clear = function () {
  this.deleteLayoutContainer();
  this.boxes && this.boxes.map(function (box) { box.clear && box.clear(); })
};

BoxesLayout.prototype.layoutContainer = function() {
  return "<div id='"+this.layoutContainerID+"'>";
};

BoxesLayout.prototype.getLayoutContainer = function() {
  return $("#"+this.layoutContainerID);
};

BoxesLayout.prototype.allBoxesHtml = function (){
  return this.boxes.map(function(box) {
    return box.getHtml();
  });
};

BoxesLayout.prototype.deleteLayoutContainer = function() {
  this.getLayoutContainer().remove();
};

BoxesLayout.prototype.renderBoxHtml = function(html, id) {
  var uniuqeID = "Box" + id,
    container,
    $html = $(html).attr("id", uniuqeID);

  if (this.boxes[id].renderTo) {
    container = $(this.boxes[id].renderTo)
  } else {
    container = this.getLayoutContainer()
  }
  container.append($html);
  this.boxes[id].selector = "#" + uniuqeID;
  if (this.boxes[id].postRender) {
    this.boxes[id].postRender(uniuqeID);
  }
};

BoxesLayout.prototype.preRender = $.noop;

BoxesLayout.prototype.render = function() {
  var self = this;

  this.deleteLayoutContainer();
  this.mainContainer.append(this.layoutContainer());

  this.preRender();

  return helpers.whenAll( this.allBoxesHtml() ).then(function(boxesHtml) {
    boxesHtml.forEach(self.renderBoxHtml.bind(self));
  });
};

module.exports = BoxesLayout;
