'use strict';

angular.module('myApp.ui', ['ngRoute'])

.config(['$routeProvider', function($routeProvider) {
  $routeProvider.when('/ui', {
    templateUrl: 'ui/ui.html',
    controller: 'UiCtrl'
  });
}])

.controller('UiCtrl', ['$scope','$timeout','$interval','$location','$http','utils',
        function($scope,$timeout,$interval,$location,$http,utils) {

            utils.ajaxindicatorstart("Fetching device infos from Particle cloud...") ;

            $scope.running = false ;
            $scope.success = false ;
            $scope.failure = false ;

            var savedState = $location.search();
            $scope.device = savedState.deviceid;
            $scope.access_token = savedState.access_token;

            if (!$scope.device) {
                $scope.response = "No device specified" ;
                return ;
            }

            if (!$scope.access_token){
                $scope.response = "No access token specified" ;
                return ;
            }

            $scope.readVariable = function(variableName,onSuccessCb,onErrorCb){
                // GET /v1/devices/0123456789abcdef01234567/temperature
                var url = "https://api.particle.io/v1/devices/:deviceId/:variablename?access_token=:access_token"
                    .replace(':deviceId', $scope.device)
                    .replace(":variablename", variableName)
                    .replace(":access_token", $scope.access_token);

                var req = {
                    method: 'GET',
                    url: url
                }

                $scope.response = "Reading...";

                $http(req).then(
                    function successCallback(response) {
                        // this callback will be called asynchronously
                        // when the response is available
                        $scope.running = false ;
                        $scope.success = true ;
                        $scope.failure = false ;
                        $scope.response = response;
                        if (onSuccessCb) onSuccessCb(response) ;

                    }, function errorCallback(response) {
                        // called asynchronously if an error occurs
                        // or server returns response with an error status.
                        $scope.running = false ;
                        $scope.success = false ;
                        $scope.failure = true ;
                        $scope.response = response;
                        if (onErrorCb) onErrorCb(response) ;
                    });
            }

            $scope.callFunction = function(functionName, functionArgs,onSuccessCb,onErrorCb) {

                console.log(functionName,functionArgs) ;

                $scope.running = true ;
                $scope.success = false ;
                $scope.failure = false ;


                if (!functionName){
                    $scope.response = "No function name specified" ;
                    return ;
                }


                if (!functionArgs){
                    $scope.response = "No function args specified";
                    return ;
                }


                var url = "https://api.particle.io/v1/devices/:deviceId/:functionName?access_token=:access_token&args=:arg"
                    .replace(':deviceId', $scope.device)
                    .replace(":functionName", functionName)
                    .replace(":access_token", $scope.access_token)
                    .replace(":arg", encodeURIComponent(functionArgs));

                var req = {
                    method: 'POST',
                    url: url,
                    /*
                     headers: {
                     "Content-type" : "application/x-www-form-urlencoded"
                     },
                     */
                    data:{
                        args:functionArgs
                    }
                }

                $scope.response = "Calling...";

                $http(req).then(
                    function successCallback(response) {
                        // this callback will be called asynchronously
                        // when the response is available
                        $scope.running = false ;
                        $scope.success = true ;
                        $scope.failure = false ;
                        $scope.response = response;
                        if (onSuccessCb) onSuccessCb(response) ;

                    }, function errorCallback(response) {
                        // called asynchronously if an error occurs
                        // or server returns response with an error status.
                        $scope.running = false ;
                        $scope.success = false ;
                        $scope.failure = true ;
                        $scope.response = response;
                        if (onErrorCb) onErrorCb(response) ;
                    });
            }

            $scope.subscribeToEvents = function(device, eventListeners){

                // GET /v1/devices/{DEVICE_ID}/events
                if (device) {
                    var eventSourceUrl = "https://api.spark.io/v1/devices/" + device + "/events/?access_token=" + $scope.access_token ;
                } else {
                    var eventSourceUrl = "https://api.spark.io/v1/events/?access_token=" + $scope.access_token ;
                }

                var eventSource = new EventSource(eventSourceUrl);

                console.log('eventsource URL',eventSourceUrl) ;

                eventSource.addEventListener('open', function(e) {
                    console.log("Opened!");
                },false);

                eventSource.addEventListener('error', function(e) {
                    console.log("Errored!",e.message);
                },false);

                eventSource.addEventListener('debugmsg',function(e){
                    console.log("debugmsg",e.data);
                });

                if (!eventListeners) return eventSource ;

                for (var eventName in eventListeners){
                    eventSource.addEventListener(eventName,eventListeners[eventName])
                }

                return eventSource ;

            }

            $scope.$on("$destroy", function(){

            });



            // preparing container for UI Elements
            $scope.uiElements = [] ;

            // we will receive ui configuration elements via "publish" from the device itself
            $scope.eventSource = $scope.subscribeToEvents($scope.device,{
                "uiConfig" : function(e){
                    //console.log('uiConfig',e) ;
                    // data: "{"data":"{0:[{'n':'status.temperature','l':'Temperature'}, {'n':'status.temperaturesetpoint','min':-10,'max':30,'l':'Set temp:','t':'slider'}]}","ttl":"60","published_at":"2015-11-08T20:54:01.811Z","coreid":"30001c000647343232363230"}"
                    var data = JSON.parse(e.data).data.replace(/'/g,'"') ;
                    console.log("data for uiConfig",data) ;
                    var dataObj = JSON.parse(data);
                    $scope.uiElements[dataObj.id] = dataObj.c ;
                    // distributing width equally for each element (based on 12 cols units of bootstrap)
                    var width = Math.floor(12/dataObj.c.length) ;
                    dataObj.c.forEach(function(uiElement){
                        uiElement.width = width ;
                        // defaulting type and label
                        if (!uiElement.t) uiElement.t = 'text' ;
                        if (!uiElement.l) uiElement.l = uiElement.n ;
                        if (uiElement.t == 'slider') {
                            if (!uiElement.min) uiElement.min = -Infinity ;
                            if (!uiElement.max) uiElement.max = Infinity ;
                            if (!uiElement.step) uiElement.step = 1 ;
                        }
                        if (uiElement.t == 'button'){
                            if (!uiElement.color) uiElement.color = "primary" ;
                        }
                    });
                }
            }) ;


            // readingVariables
            $scope.readingStatusVariableTentativeCount = 0 ;
            $scope.readStatusVariable = function(cb){
                if (!cb) cb = function(){} ;
                $scope.readingStatusVariableTentativeCount++ ;
                if ($scope.readingStatusVariableTentativeCount > 10){
                    $scope.readingStatusVariableTentativeCount = 0 ;
                    console.log("too much $scope.readingStatusVariableTentativeCount") ;
                }
                $scope.readVariable(
                    "status",
                    function(okResponse){
                        $scope.readingStatusVariableTentativeCount = 0 ;
                        console.log("ok var",okResponse) ;
                        var data = JSON.parse(okResponse.data.result);
                        $scope.uiElements.forEach(function(uiElementRow){
                            uiElementRow.forEach(function(uiElement){
                                var varName = uiElement.n ;
                                if (uiElement.value != data[varName]){
                                    uiElement.changed = true ;
                                    uiElement.value = data[varName] ;
                                }
                            }) ;
                        }) ;
                        $timeout(function(){
                            $scope.resetChanged() ;
                        },3000) ;
                        cb(okResponse) ;
                    },
                    function(nokResponse){
                        cb(nokResponse) ;
                        console.log("nok var",nokResponse) ;
                    }
                ) ;
            }



            // requesting UI Configuration
            $scope.callFunction(
                'message',
                'getuiconfig:now',
                function(successResponse){
                    console.log('ok', successResponse) ;
                    $scope.readStatusVariable(function(){
                        utils.ajaxindicatorstop() ;
                    });
                },
                function(failureResponse){
                    console.log("nok", failureResponse) ;
                }
            ) ;


            // interface callbacks


            $scope.resetChanged = function(){
                $scope.uiElements.forEach(function(uiElementRow){
                    uiElementRow.forEach(function(uiElement){
                        uiElement.changed = false ;
                    }) ;
                }) ;
            }

            $scope.updateValue = function(uiElement){
                var functionArgs = uiElement.n + ":" + uiElement.value ;
                var functionName = 'message' ;
                $scope.callFunction(
                    functionName,
                    functionArgs,
                    function(okStatus){
                        console.log('ok',okStatus) ;
                        //$scope.readStatusVariable();
                    },
                    function(nokStatus){
                        console.log("nok",nokStatus) ;
                    }
                )
            }

            $scope.buttonClick = function(uiElement){
                var functionArgs = uiElement.m ;
                var functionName = 'message' ;
                $scope.callFunction(
                    functionName,
                    functionArgs,
                    function(okStatus){
                        console.log('ok',okStatus) ;
                        $scope.readStatusVariable();
                    },
                    function(nokStatus){
                        console.log("nok",nokStatus) ;
                    }
                )
            }

            $interval(function(){
                $scope.readStatusVariable() ;
            },10000)


}]);