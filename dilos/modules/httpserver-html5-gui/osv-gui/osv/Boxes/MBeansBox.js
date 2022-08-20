var BaseBox = require("./BaseBox"),
    Jolokia = require("../API/Jolokia");
    
function MBeansBox() {
}

MBeansBox.prototype = Object.create(BaseBox.prototype);

MBeansBox.prototype.template = "/osv/templates/boxes/MBeansBox.html";

MBeansBox.prototype.fetchData = function () {
  return Jolokia.MBeans.tree().then(function (res) {
    console.log(res);
    return res;
  });
};

MBeansBox.prototype.toggleList = function(event) {
  var $el = $(event.currentTarget),
    $nextEl = $el.next();

  if ($nextEl.is("ul")) {
    $nextEl.toggleClass("hidden")
    $el.toggleClass("opened")
  }
};

MBeansBox.prototype.postRender = function() {
  $(document).on("click", ".MBeans li", this.toggleList);
};

module.exports = MBeansBox;
