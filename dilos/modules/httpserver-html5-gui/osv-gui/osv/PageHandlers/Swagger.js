function Swagger() {}

Swagger.prototype.handler = function () {
  var self = this;
  var $iframe = $("<iframe style='border:none' id='osvContainer' src='/dashboard_static/swagger/'>")
  $("#main").append($iframe);
  $(".main_tabsholder").hide();
  $("#osvContainer").css("top", "50px");
  $(document).on("runRoute", function () {
    self.clean();
  });

};

Swagger.prototype.clean = function () {
  $(".main_tabsholder").show();
  $("#osvContainer").css("top", "87px");
};

module.exports = Swagger;