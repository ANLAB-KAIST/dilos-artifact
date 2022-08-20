var BaseBox = require("./BaseBox");

function StaticBox() {
}

StaticBox.prototype = Object.create(BaseBox.prototype);

StaticBox.prototype.template = "/osv/templates/boxes/StaticBox.html";

module.exports = StaticBox;
