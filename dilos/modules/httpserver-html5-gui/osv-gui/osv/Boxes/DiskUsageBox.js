var BaseBox = require("./BaseBox"),
    FS = require("../API/FS"),
    SideTextGraphBox = require("./SideTextGraphBox"),
    helpers = require("../helpers");

function DiskUsageBox() {
  this.sideText = SideTextGraphBox.prototype;
}

DiskUsageBox.prototype = Object.create(BaseBox.prototype);

DiskUsageBox.prototype.template = "/osv/templates/boxes/SideTextGraphBox.html";

DiskUsageBox.prototype.sideTextTemplate = "/osv/templates/boxes/SideText.html"

DiskUsageBox.prototype.getSideText = function (df) {
  return [
    {
      label: "File System",
      value: df.filesystem,
      unit: ""
    },

    {
      label: "Size",
      value: helpers.humanReadableByteSize(df.btotal),
      unit: ""
    },

    {
      label: "Used",
      value: helpers.humanReadableByteSize(df.btotal - df.bfree),
      unit: "",
      background: "#2553DA"
    },

    {
      label: "Available",
      value: helpers.humanReadableByteSize(df.bfree),
      unit: "",
      background: "#01CB34"
    },
    
  ];
};

DiskUsageBox.prototype.fetchData = function() {
  var self = this;
  return $.Deferred().resolve({ title: "Disk Usage" });
};

DiskUsageBox.prototype.postRender = function() {
  var self = this;

  $.when(FS.free(), FS.df()).then(function (free, df) {
    
    var sideTextHtml = self.sideText.getSideTextTemplate()(self.getSideText(df[0]));
    $(self.selector).find('.sideTextContainer').html(sideTextHtml);

    jQuery.jqplot (self.selector.replace("#","") + " .jqplot",  
      [
        [            
          ['Free', free], 
          ['Full', 100-free]
        ]
      ], 
      { 
        seriesColors: [ "#00cc33", "#2553da" ],

         grid: {
            drawGridLines: true,        // wether to draw lines across the grid or not.
            gridLineColor: '#CCCCCC',    // *Color of the grid lines.
            background: '#FFF',      // CSS color spec for background color of grid.
            borderColor: '#CCCCCC',     // CSS color spec for border around grid.
            borderWidth: 0.1,           // pixel width of border around grid.
            shadow: false,               // draw a shadow for grid.
            renderer: $.jqplot.CanvasGridRenderer,  // renderer to use to draw the grid.
            rendererOptions: {}         // options to pass to the renderer.  Note, the default
                                        // CanvasGridRenderer takes no additional options.
        },
        seriesDefaults: {
          renderer: jQuery.jqplot.PieRenderer, 
          rendererOptions: {
            showDataLabels: true
          }
        }, 
        legend: { show:true, location: 'e' }
      }
    );

  });
};

module.exports = DiskUsageBox;
