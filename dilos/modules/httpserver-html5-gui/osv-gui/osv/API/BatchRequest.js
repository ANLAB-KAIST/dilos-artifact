var Settings = require("../Settings");

function BatchRequests (basePath, batchAPIPath, refreshRate) {
  this.basePath = basePath;
  this.url = basePath + batchAPIPath;
  this.queue = [];
  this.promises = [];
  this.interval = setInterval(this.commitRequestsQueue.bind(this), refreshRate)
};

BatchRequests.prototype.addRequest = function (request, toBegining) {
  var promise = $.Deferred();
  if (toBegining) {
    this.queue.unshift(request);
    this.promises.unshift(promise);
  } else {
    this.queue.push(request);
    this.promises.push(promise);
  }
  return promise;
}

BatchRequests.prototype.getFormatedQueue = function () {
  return this.queue.map(function (request) {
    return {"method": request.method, "relative_url": request.path}
  });
};

BatchRequests.prototype.resetQueue = function () {
  this.queue = [];  
  this.promises = [];
};

BatchRequests.prototype.resolve = function (promises, responses) {
  responses.forEach(function (response, idx) {
    if (response.code == 200) {
      promises[idx].resolve(response.body);
    } else {
      promises[idx].reject(response.code)        
    }
  });
};

BatchRequests.prototype.reject = function (promises, error) {
  promises.forEach(function (promise) {
    promise.reject(err)
  });
};

BatchRequests.prototype.getRequests = function () {   
  return JSON.stringify(this.queue.map(function (request) {    
      return {"method":request.method, "relative_url":request.path}   
  }))    
};

BatchRequests.prototype.getFormData = function () {    
  var formData = new FormData(),   
      requests = this.getRequests();   
 
  formData.append("batch", requests);    
  return formData;   
};   
 
BatchRequests.prototype.postData = function (formData) {   
  return $.ajax({    
    url: this.url,   
    data: formData,    
    cache: false,    
    contentType: false,    
    processData: false,    
    type: 'POST',    
  });    
};   
 
BatchRequests.prototype.commitRequestsQueue = function () {    
  var self = this,   
      queue = this.queue,    
      promises = this.promises,    
      formData = this.getFormData();   
       
  if (queue.length == 0) return;   
  this.resetQueue();   
  this.postData(formData)    
  .then(function (responses) {
    if(typeof(responses) === "string") {
      responses = JSON.parse(responses.replace(/<h1>We didn't find the page you were looking for<\/h1>/g, 404));
    }
    responses = responses.map(function (response) {
      return response.generatedTimestamp = Date.now(), response;
    });   
    if (window.globalPause) {    
      $(document).on("play", function () {   
        self.resolve(promises, responses);   
      })   
    } else {   
        self.resolve(promises, responses);   
    }    
  })   
  .fail(function (error) {   
    self.reject(promises, error)     
  });    
};   
 
BatchRequests.prototype.ajax = function (method, path, priorityRequest) {   
  return this.addRequest({   
    method: method,    
    path: path   
  }, priorityRequest);    
};

BatchRequests.prototype.get = function (path) {    
  return this.ajax("GET", path);   
};   
 
BatchRequests.prototype.post = function (path, data) {   
  data = "?" + $.map(data, function (value, key) {   
    return key + "=" + value;    
  }).join("&")   
  return this.ajax("POST", path + data);   
};   
 
BatchRequests.prototype.delete = function (path) {   
  return this.ajax("DELETE", path);    
};   

var singleton = new BatchRequests(Settings.BasePath, "/api/batch", Settings.DataFetchingRate);

module.exports = singleton;
