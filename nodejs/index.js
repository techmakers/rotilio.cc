/**
 * Created by cashbit on 15/01/16.
 */


var access_token = "<YOUR_ACCESS_TOKEN_HERE>" ;


// https://api.spark.io/v1/devices/30001c000647343232363230/status?access_token=<YOUR_ACCESS_TOKEN_HERE>

console.log("Run..");

var https = require('https');

https.get({
    host: 'api.spark.io',
    path: '/v1/devices/23001d001447343339383037/status?access_token=' + access_token
}, function (response) {
    // Continuously update stream with data
    var body = '';
    response.on('data', function (d) {
        body += d;
    });
    response.on('end', function () {

        // Data reception is done, do whatever with it!
        console.log(body) ;
    });
});




var EventSource = require('eventsource');

var eventSourceUrl = "https://api.spark.io/v1/devices/23001d001447343339383037/events/?access_token=" + access_token ;

var es = new EventSource(eventSourceUrl);
es.onmessage = function(e) {
    console.log(e.data);
};
es.onerror = function(err) {
    console.log(err);
};


https.request({
    method: 'post',
    host: 'api.spark.io',
    path: '/v1/devices/23001d001447343339383037/message?access_token=' + access_token + '&args=getuiconfig:now'
}, function (response) {
    // Continuously update stream with data
    var body = '';
    response.on('data', function (d) {
        body += d;
    });
    response.on('end', function () {

        // Data reception is done, do whatever with it!
        console.log(body) ;
    });
});