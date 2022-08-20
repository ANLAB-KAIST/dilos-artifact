var BaseHandler = require("./PageHandlers/BaseHandler"),
    MainHandler = require("./PageHandlers/Dashboard/Main"),
    ThreadsHandler = require("./PageHandlers/Dashboard/Threads"),
    TracesHandler = require("./PageHandlers/Dashboard/Traces"),
    JVMHandler = require("./PageHandlers/Dashboard/JVM"),
    CassandraHandler = require("./PageHandlers/Dashboard/Cassandra"),
    TomcatHandler = require("./PageHandlers/Dashboard/Tomcat"),
    SwaggerHandler = require("./PageHandlers/Swagger");


module.exports = Davis(function() {
  var
    self = this,
    basePaths = ["", "/dashboard"] 
    baseHandler = new BaseHandler(),
    mainHandler = new MainHandler(),
    threadsHandler = new ThreadsHandler(),
    tracesHandler = new TracesHandler(),
    jvmHandler = new JVMHandler(),
    cassandraHandler = new CassandraHandler(),
    tomcatHandler = new TomcatHandler(),
    runRoute = new CustomEvent('runRoute'),
    swaggerHandler = new SwaggerHandler();

  this.configure(function() {
    this.generateRequestOnPageLoad = true;
  });

  this.get("/", function() {
    mainHandler.handler();
  });
  
  this.get("/dashboard/", function() {
	  mainHandler.handler();
  });
  basePaths.forEach(function (basePath) {
  

    self.get(basePath + "/threads/", function() {
      threadsHandler.handler();
    });

    self.get(basePath + "/traces/", function() {
      tracesHandler.handler();
    });

    self.get(basePath + "/cassandra/", function() {
      cassandraHandler.handler();
    });

    self.get(basePath + "/jvm/", function() {
      jvmHandler.handler();
    });

    self.get(basePath + "/tomcat/", function() {
      tomcatHandler.handler();
    });

    self.get(basePath + "/swagger/", function() {
      swaggerHandler.handler();
    });

  })
    
  this.bind('runRoute', function (data) {
    runRoute.initCustomEvent('runRoute', true, true, data);
    document.dispatchEvent(runRoute);
  })
});
