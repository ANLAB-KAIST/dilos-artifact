var Boxes = require("../../Boxes/Boxes"),
    BoxesLayout = require("../../Layouts/BoxesLayout");

function JVM() {
  this.subscribe();
}

JVM.prototype.handler = function() {
  this.MBeansBox = new Boxes.MBeansBox();
  this.MBeansAttributesBox = new Boxes.MBeansAttributesBox();
  this.layout = new BoxesLayout([ 
    new Boxes.JVMStaticInfo(), new Boxes.GCGraph(),
    new Boxes.HeapMemoryUsage(), new Boxes.MemoryPoolGraph(),
    this.MBeansBox, this.MBeansAttributesBox
  ]);
  this.layout.render();
};

JVM.prototype.MBeansClicked = function(event) {
  var $li = $(event.currentTarget),
    name = $li.attr("data-rawName");

  $(".MBeans .selected").removeClass("selected");
  $li.addClass("selected");
  this.MBeansAttributesBox.name = name;
  this.MBeansAttributesBox.refresh();
};

JVM.prototype.subscribe = function() {
  $(document).on("click", ".MBeans [data-hasAttr]", this.MBeansClicked.bind(this));
};

module.exports = JVM;
