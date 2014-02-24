$(function(){

  // Handle selection of BaudRate
  var baudRate = $('#baudRate');
  // ToDo: handle the onChange of baudRate

  // Get a listing of available ports and add them to the selector
  var chooser = $('#choosePort').empty();
  var onGetDevices = function(ports) {
    $('<option value=nope selected>Select the port</option>').appendTo(chooser);
    for (var i=0; i<ports.length; i++) {
      console.log(ports[i].path);
      $('<option>'+ports[i].path+'</option>').appendTo(chooser);
    }
  }
  chrome.serial.getDevices(onGetDevices);
  
  // Make the Selector onChange event handler that makes the connection
  chooser.change(function(){
    if (chooser.val() != "nope")  // "nope" is the default value of the "select a port" option
      //chrome.serial.connect(chooser.val(), {bitrate: parseInt(baudRate.val())}, onConnect);
      chrome.serial.connect(chooser.val(), {bitrate: 4800}, onConnect);
  });
  
  // Do this after the port connection is made
  var connectionId;
  var onConnect = function(connectionInfo) {
     // The serial port has been opened. Save its id to use later.
    connectionId = connectionInfo.connectionId;
    // Do whatever you need to do with the opened port.
  }

  // do something with the input from the port
  var textarea = $('#serialConsole');    
  var onLineReceived = function(strReceived) {
    textarea.val(strReceived+"\n"+textarea.val());
  };
  var stringReceived = '';
  var onReceiveCallback = function(info) {
    if (info.connectionId == connectionId && info.data) {
      var str = ab2str(info.data);
      if (str.charAt(str.length-1) === '\n') {
        stringReceived += str.substring(0, str.length-1);
        onLineReceived(stringReceived);
        stringReceived = '';
      } else {
        stringReceived += str;
      }
    }
  };
  chrome.serial.onReceive.addListener(onReceiveCallback);

  function ab2str(buf) {
    return String.fromCharCode.apply(null, new Uint8Array(buf));
  }
  function str2ab(str) {
    var buf = new ArrayBuffer(str.length*2); // 2 bytes for each char
    var bufView = new Uint8Array(buf);
    for (var i=0, strLen=str.length; i<strLen; i++) {
       bufView[i] = str.charCodeAt(i);
    }
    return buf;
  }

});
