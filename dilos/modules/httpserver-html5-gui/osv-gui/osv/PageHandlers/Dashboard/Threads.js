var ThreadsLayout = require("../../Layouts/ThreadsLayout");

function Threads() {
}

Threads.prototype.handler = function() {
	this.layout = this.layout || new ThreadsLayout();
	this.layout.render();
};

module.exports = Threads;
