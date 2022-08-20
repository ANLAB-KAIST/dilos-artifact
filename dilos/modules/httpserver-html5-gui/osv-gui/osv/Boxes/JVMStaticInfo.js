var JVM = require("../API/JVM"),
    helpers = require("../helpers"),
    BaseBox = require("./BaseBox");

function JVMStaticInfo() {
}

JVMStaticInfo.prototype = Object.create(BaseBox.prototype);

JVMStaticInfo.prototype.template = "/osv/templates/boxes/StaticBox.html";

JVMStaticInfo.prototype.fetchData = function () {
  return $.when(
    JVM.version(),
    JVM.HeapMemoryUsed(),
    JVM.classesCount(),
    JVM.threadsCount()
  ).then(function (version, heapyMemoryUsed, classesCount, threadsCount) {
    return [
      { key: "JVM Version", value: version },
      { key: "Heap Memory Used", value: helpers.humanReadableByteSize(heapyMemoryUsed) },
      { key: "Total Loaded Classes", value: classesCount },
      { key: "Threads Count", value: threadsCount }
    ]
  });
}

module.exports = JVMStaticInfo;
