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
      chrome.serial.connect(chooser.val(), {bitrate: 9600}, onConnect);
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
    try {
      showData( NMEA.parse(strReceived) );
    } catch(err){
      // Todo: do something with the parsing error
      console.log(err.message);
    }
    textarea.val(strReceived+"\n"+textarea.val());
  };
  var stringReceived = '';
  var onReceiveCallback = function(info) {
    if (info.connectionId == connectionId && info.data) {
      var str = ab2str(info.data);
      if (str.charAt(str.length-1) === '\n') {
        stringReceived += str.substring(0, str.length-1);
        var parts = stringReceived.split('$');
        onLineReceived( '$'+parts.pop() );
        if (parts.length) 
            stringReceived = '$'+parts.join('$'); 
//        onLineReceived( stringReceived.substr(0, stringReceived.indexOf('\n')+1) ); 
//        stringReceived = stringReceived.substr(stringReceived.indexOf('\n')+1, stringReceived.length);
      } else {
        stringReceived += str;
      }
    }
  };
  chrome.serial.onReceive.addListener(onReceiveCallback);

  var infoPane = $('#infoPane');
  var infoList = $('<dl></dl>').appendTo(infoPane);
  var pressure = $('<dt>pressure</dt><dd></dd>').hide().appendTo(infoList);
  var airTemp = $('<dt>air temperature</dt><dd></dd>').hide().appendTo(infoList);
  var humidity = $('<dt>humidity</dt><dd></dd>').hide().appendTo(infoList);
  var dewPoint = $('<dt>dew point</dt><dd></dd>').hide().appendTo(infoList);
  var windDir = $('<dt>wind direction</dt><dd></dd>').hide().appendTo(infoList);
  var windSpeed = $('<dt>wind speed</dt><dd></dd>').hide().appendTo(infoList);
  var windSpeed2 = $('<dt>wind speed #2</dt><dd></dd>').hide().appendTo(infoList);
  var windAngle = $('<dt>wind angle</dt><dd></dd>').hide().appendTo(infoList);
  var windReference = $('<dt>wind reference</dt><dd></dd>').hide().appendTo(infoList);
  var turnRate = $('<dt>turn rate</dt><dd></dd>').hide().appendTo(infoList);
  var windChill = $('<dt>wind chill</dt><dd></dd>').hide().appendTo(infoList);
  var baroPressure = $('<dt>barometric pressure</dt><dd></dd>').hide().appendTo(infoList);
  var pitch = $('<dt>pitch</dt><dd></dd>').hide().appendTo(infoList);
  var roll = $('<dt>roll</dt><dd></dd>').hide().appendTo(infoList);
  var course = $('<dt>course over ground</dt><dd></dd>').hide().appendTo(infoList);
  var speed = $('<dt>speed over ground</dt><dd></dd>').hide().appendTo(infoList);
  var time = $('<dt>time</dt><dd></dd>').hide().appendTo(infoList);

            var options = {
                            chart: {
                              zoomType: 'x',
                              renderTo: 'container'
                            }, 
                            title: {text: 'Temp'},
                            subtitle: {text: document.ontouchstart === undefined?
                                          'Click and drag in the plot area to zoom in':
                                           'Pinch the chart to zoom in'
                                         },
                            xAxis: {
                                      title: {text: null},
                                      type: 'datetime'
                                    },
                            yAxis: [{ // left y axis
                                        title: {text: 'Temperature (C)'},
                                        labels: {align: 'left', x: 3, y: 16,
                                                formatter: function() {
                                        return Highcharts.numberFormat(this.value, 0);}
                                    },
                                    showFirstLabel: false}, 
                                    { // right y axis
                                        linkedTo: 0, gridLineWidth: 0, opposite: true,
                                        title: {text: 'Temperature (C)'},
                                                labels: {align: 'right', x: -3, y: 16,
                                                    formatter: function() {
                                        return Highcharts.numberFormat(this.value, 0);}
                                    },
                                    showFirstLabel: false
                            }],
//                            legend: {align: 'left', verticalAlign: 'top', y: 20,
//                                    floating: true, borderWidth: 0},
                            tooltip: {shared: true, crosshairs: true},
//                            plotOptions: {  series: {cursor: 'pointer',
//                                                    point: {events: {
//                                                    click: function() {
//                                                    hs.htmlExpand(null, {
//                                                    pageOrigin: {
//                                                        x: this.pageX, 
//                                                        y: this.pageY
//                                                    },
//                                                    headingText: this.series.name,
//                                                    maincontentText: Highcharts.dateFormat('%A, %b %e, %Y', this.x) +':<br/> '+ 
//                                                    this.y +'(℉)',
//                                                    width: 200});
//                                                    }}},
//                            marker: {lineWidth: 1}}},
                            series: [   {name: 'Air Temp'}
                                    ]
            };
    var chart = new Highcharts.Chart(options);

  var showData = function(obj){
    if(obj && (!obj['status'] || obj['status'] == 'A')) {
      if(obj.pressureMercury != null && obj.pressureBars != null)
        pressure.show().filter('dd').html(obj.pressureMercury+' ('+obj.pressureBars+')');
      if(obj.airTemp != null) {
        airTemp.show().filter('dd').html(obj.airTemp);
        chart.series[0].addPoint([new Date().getTime(), obj.airTemp]);
      }
      if(obj.humidity != null)
        humidity.show().filter('dd').html(obj.humidity);
      if(obj.dewPoint != null)
        dewPoint.show().filter('dd').html(obj.dewPoint);
      if(obj.windDirTrue != null  && obj.windDirMag != null)
        windDir.show().filter('dd').html(obj.windDirTrue+' ('+obj.windDirMag+' magnetic)');
      if(obj.windSpeedKnots != null)
        windSpeed.show().filter('dd').html(obj.windSpeedKnots+' knots');
      if(obj.windSpeedKnots2 != null)
        windSpeed2.show().filter('dd').html(obj.windSpeedKnots2+' knots ('+obj.windSpeedMPS+' meters/second)');
      if(obj.windAngle != null)
        windAngle.show().filter('dd').html(obj.windAngle);
      if(obj.windReference != null)
        windReference.show().filter('dd').html(obj.windReference);
      if(obj.turnRate != null)
        turnRate.show().filter('dd').html(obj.turnRate);
      if(obj.windChillRelative != null)
        windChill.show().filter('dd').html(obj.windChillRelative+'('+obj.windChillTheoretical+')');
      if(obj.barometricPressure != null)
        baroPressure.show().filter('dd').html(obj.barometricPressure);        
      if(obj.pitch != null)
        pitch.show().filter('dd').html(obj.pitch);
      if(obj.roll != null)
        roll.show().filter('dd').html(obj.roll);
      if(obj.groundCourseTrue != null)
        course.show().filter('dd').html(obj.groundCourseTrue+'('+obj.groundCourseMagnetic+')');
      if(obj.speedKnots != null)
        speed.show().filter('dd').html(obj.speedKmHr+'('+obj.speedKmHr+')');
      if(obj.time != null)
        time.show().filter('dd').html(obj.time);
    }
  };

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
