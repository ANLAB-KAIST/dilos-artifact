var apiGETCall = require("../helpers").apiGETCall, 
  df = apiGETCall("/fs/df/"),
  free,
  data;

free = function() {
  return df().then(function (res) {
    return (res[0].bfree / res[0].btotal) * 100;
  })
};

module.exports = { 
  free: free,
  df: df
};
