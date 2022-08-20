var GraphBox = require("./GraphBox"),
    OS = require("../API/OS"),
    Settings = require("../Settings");

function CPUBox() {
  GraphBox.call(this, arguments)
}

CPUBox.prototype = Object.create(GraphBox.prototype);

CPUBox.prototype.cpus = [];

CPUBox.prototype.title = "CPU";

CPUBox.prototype.extraSettings = function() {
  return {
    grid: {
        drawGridLines: true,        // wether to draw lines across the grid or not.
        gridLineColor: '#CCCCCC',    // *Color of the grid lines.
        background: '#FDFDFD',      // CSS color spec for background color of grid.
        borderColor: '#CCCCCC',     // CSS color spec for border around grid.
        borderWidth: 0.1,           // pixel width of border around grid.
        shadow: false,               // draw a shadow for grid.
        renderer: $.jqplot.CanvasGridRenderer,  // renderer to use to draw the grid.
        rendererOptions: {}         // options to pass to the renderer.  Note, the default
                                    // CanvasGridRenderer takes no additional options.
    },
    axes: {
      xaxis: {
        renderer: $.jqplot.DateAxisRenderer,
        tickOptions: {
          formatString: "%H:%M:%S"
        }
      }
    },
  series: [{
        label: "CPU Usage %"
    }]
  }
};

CPUBox.prototype.fetchData = function() {
  var cpuData = OS.CPUAverage();
  var plots = cpuData.slice(-1 * Settings.Graph.MaxTicks )
  if (plots.length == 0) {
    plots = [ null ];
  } 
  return $.Deferred().resolve([ plots ]);
};

module.exports = CPUBox;
