var BaseBox = require("./BaseBox"),
    Settings = require("../Settings");

function GraphBox() {
  this.subscribe();
}

GraphBox.prototype = new BaseBox();

GraphBox.prototype.subscribe = function () {
  $(document).on("play", this.play.bind(this));
  $(document).on("pause", this.pause.bind(this));

}
GraphBox.prototype.baseSettings = function() {
  return {
    seriesColors: [ "#3200FF", "#FF9900", "#C601CB", "#17FC81", "#F1FF00" ],
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
    axisDefaults: {
      showLabel: false,
      labelOptions: {
        show: false,
        fontSize: '14px'
      }
    },
    seriesDefaults: { 
      lineWidth: 0.5,
      markerOptions: {
        show: false,
        size: 0.2
      },
    },
    highlighter: {
          show: true,
          sizeAdjust: 7.5
      },
      legend: {
        show: false,
          location: "nw",
          xoffset: 12,
          yoffset: 12
      }
  }
};

GraphBox.prototype.extraSettings = function() {
  return {};
};

GraphBox.prototype.getSettings = function() {
  var settings = $.extend(this.baseSettings(), this.extraSettings())
  if (settings.series) {
    settings.series = settings.series.map(function (series, idx) {
      series.color = settings.seriesColors[idx];
      return series;      
    })
  }
  return settings;
};

GraphBox.prototype.title = "Graph";

GraphBox.prototype.template = "/osv/templates/boxes/GraphBox.html";

GraphBox.prototype.getHtml = function() {
  var template = this.getTemplate(),
    context = { title: this.title, settings: this.getSettings() },
    html = template(context);

  return html;
};

GraphBox.prototype.fetchData = function() {
  return $.Deferred().resolve([ [ null ] ]);
};

GraphBox.prototype.renderGraph = function(selector, setATimeout) {
  var self = this,
    settings = self.getSettings();

  selector = selector || this.selector;
  this.selector = selector;
  this.fetchData().then(function(data) {
    if (self.onUpdate) self.onUpdate();
    if (self.plot) {
      self.plot.destroy();
    }
    
    if (data[0][0]) settings.axes.xaxis.min = data[0][0][0];
    if (data[data.length-1][0]) settings.axes.xaxis.max = data[0][data[0].length-1][0];
    
    self.plot = $.jqplot(selector + " .jqplot", data, settings);
  });
  this.isViewed = true;
  if (setATimeout !== false && !window.globalPause) {
    this.timeout = setTimeout(function() { self.renderGraph(selector) }, Settings.DataFetchingRate);
  }
};

GraphBox.prototype.clear = function () {
  clearTimeout(this.timeout)
  $(this.selector).remove();
  this.isViewed = false;
};

GraphBox.prototype.pause = function () {
 // clearTimeout(this.timeout);
};

GraphBox.prototype.play = function () {
 if (this.selector && this.isViewed) this.renderGraph(this.selector, true);
};

GraphBox.prototype.postRender = function(selector) {
  this.selector = selector;
  this.renderGraph(selector);
};

module.exports = GraphBox;
