var BaseBox = require("./BaseBox"),
    Jolokia = require("../API/Jolokia");

function MBeansAttributesBox() {
  this.subscribe();
}

MBeansAttributesBox.prototype = Object.create(BaseBox.prototype);

MBeansAttributesBox.prototype.template = "/osv/templates/boxes/MBeansAttributesBox.html";

MBeansAttributesBox.prototype.fetchData = function() {
  if (!this.name) return $.Deferred().resolve([])
  return Jolokia.MBeans.attributes(this.name);
};

MBeansAttributesBox.prototype.nextTabularSlide = function(clickEvent) {
  var $btn = $(clickEvent.currentTarget),
    $slideNum = $btn.parent().find(".slideNum"),
    currentSlide = $slideNum.html() - 1,
    nextSlideNum = currentSlide + 1,
    $td = $btn.parents("td"),
    $currentSlide = $td.find("table[data-idx='" + currentSlide + "']"),
    $nextSlide = $td.find("table[data-idx='" + nextSlideNum + "']");
  
  if ($nextSlide.length > 0) {
    $currentSlide.addClass("hidden");
    $nextSlide.removeClass("hidden");
    $slideNum.html(nextSlideNum + 1)
  } else {
    $slideNum.html("0");
    $currentSlide.addClass("hidden");
    this.nextTabularSlide(clickEvent);
  }
};

MBeansAttributesBox.prototype.previousTabularSlide = function(clickEvent) {
  var $btn = $(clickEvent.currentTarget),
    $td = $btn.parents("td"),
    $slideNum = $btn.parent().find(".slideNum"),
    currentSlide = $td.find("table:not(.hidden)").attr("data-idx"),
    nextSlideNum = currentSlide - 1,
    lastSlideNum = $td.find(".total").html() - 1,
    $currentSlide = $td.find("table[data-idx='" + currentSlide + "']"),
    $nextSlide = $td.find("table[data-idx='" + nextSlideNum + "']"),
    $lastSlide = $td.find("table[data-idx='" + lastSlideNum + "']");

  if ($nextSlide.length > 0) {
    $currentSlide.addClass("hidden");
    $nextSlide.removeClass("hidden");
    $slideNum.html(nextSlideNum + 1)
  } else {
    $currentSlide.addClass("hidden");
    $lastSlide.removeClass("hidden")
    $slideNum.html(lastSlideNum + 1)

  }

};

MBeansAttributesBox.prototype.subscribe = function() {
  $(document).on("click", ".tabluarDataControllers .next", this.nextTabularSlide.bind(this));
  $(document).on("click", ".tabluarDataControllers .previous", this.previousTabularSlide.bind(this));
}

module.exports = MBeansAttributesBox;
