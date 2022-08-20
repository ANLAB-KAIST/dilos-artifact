var SideTextGraphBox = require("./SideTextGraphBox"),
    helpers = require("../helpers"),
    OS = require("../API/OS");
    
function MemoryBox() {
  SideTextGraphBox.call(this, arguments);
}

MemoryBox.prototype = Object.create(SideTextGraphBox.prototype);

MemoryBox.prototype.title = "Memory";

MemoryBox.prototype.extraSettings = function() {
  return {
    axes: {
      xaxis: {
        renderer: $.jqplot.DateAxisRenderer,
        tickOptions: {
          formatString: "%H:%M:%S"
        }
      },
      yaxis: {
        max: this.total,
        tickOptions: {
          formatter: function(foramt, val) {
            return helpers.humanReadableByteSize(val * Math.pow(1024, 2),2);
          }
        }
      }
    },
    series: [
      {
        label: "Free",
      },

      {
        label: "Total"
      }

    ],
  }
};

MemoryBox.prototype.getSideText = function () {
  return [
    {
      label: "Total",
      value: helpers.humanReadableByteSize(this.total * Math.pow(1024, 2),2),
      unit: ""
    },

    {
      label: "Free",
      value: helpers.humanReadableByteSize(this.free * Math.pow(1024, 2),2),
      unit: ""
    },

  ]
};

MemoryBox.prototype.fetchData = function() {
  var MemoryHistory = OS.Memory.History,
    free = MemoryHistory.free(),
    total = MemoryHistory.total();

  this.total = total[ total.length - 1][1];
  this.free = free[ free.length - 1][1];
  // If there was no data fetched yet, the graph will break the whole application.
  // this is a workaround.
  if (free.length === 0) {
    free = [ null ];
  }
  if (total.length === 0) {
    total = [ null ];
  }

  return $.Deferred().resolve([ free ]);
};

module.exports = MemoryBox;
