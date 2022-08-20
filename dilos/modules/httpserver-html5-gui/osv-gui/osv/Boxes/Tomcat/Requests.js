var RequestsAPI = require("../../API/Applications/Tomcat/Requests"),
    GraphBox = require("../GraphBox"),
    helpers = require("../../helpers");

function Requests() {
  GraphBox.call(this, arguments);
}

Requests.prototype = Object.create(GraphBox.prototype);

Requests.prototype.title = "Requests";

Requests.prototype.extraSettings = function() {
  return {
    axes: {
      xaxis: {
        renderer: $.jqplot.DateAxisRenderer,
        tickOptions: {
          formatString: "%H:%M:%S"
        }
      },
    },
    series: RequestsAPI.getLabels().map(function (label) {
      return {
        label: label
      }
    }),
  }
};

Requests.prototype.fetchData = function() {
  var data = RequestsAPI.getPlots();
  return $.Deferred().resolve(data);
    
};

module.exports = Requests;
