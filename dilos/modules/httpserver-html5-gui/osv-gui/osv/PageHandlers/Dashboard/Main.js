var Boxes = require("../../Boxes/Boxes"),
    BoxesLayout = require("../../Layouts/BoxesLayout");

function Main() {
}

Main.prototype.handler = function() {
  this.layout = new BoxesLayout([
    new Boxes.StaticInfo(), new Boxes.MemoryBox(),
    new Boxes.CPUBox(), new Boxes.CPUTableBox(),
    new Boxes.DiskUsageBox()
  ]);
  this.layout.render();
};

module.exports = Main;
