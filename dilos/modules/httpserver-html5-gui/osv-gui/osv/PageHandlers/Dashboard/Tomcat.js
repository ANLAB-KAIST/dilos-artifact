var Boxes = require("../../Boxes/Tomcat/Boxes"),
    BoxesLayout = require("../../Layouts/BoxesLayout");

function Tomcat() {
}

Tomcat.prototype.handler = function() {
  this.layout = new BoxesLayout([
    new Boxes.Threads(), new Boxes.Requests(),
    new Boxes.Sessions()
  ]);
  this.layout.render();
};

module.exports = Tomcat;
