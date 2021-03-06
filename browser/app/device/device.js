'use strict';

angular.module('myApp.device', ['ngRoute'])

.config(['$routeProvider', function($routeProvider) {
  $routeProvider.when('/device', {
    templateUrl: 'device/device.html',
    controller: 'DeviceCtrl'
  });
}])

.controller('DeviceCtrl', ['$scope','$timeout','$interval','$location','$rootScope','utils',
        function($scope,$timeout,$interval,$location,$rootScope,utils) {

            var savedState = $location.search();
            $scope.access_token = savedState.access_token ;

            if (!$scope.access_token) {
                $location.path("/") ;
                return ;
            }

            $rootScope.access_token = $scope.access_token ;

            $scope.devices = [] ;

            $scope.deviceVariables = {} ;

            $scope.rotilio = new rotiliocc({
                admittedDevices : [savedState.deviceid],
                deviceAdded: function(device){
                    $timeout(function(){
                        $scope.devices.push(device) ;
                        $scope.subscribeToEvents(device) ;
                    },0)
                },
                variableChanged :function(data){
                    $timeout(function(){
                        // is json ?
                        try{
                            var jsonObj = JSON.parse(data.result) ;
                            console.log("changed",jsonObj) ;
                            for (var k in jsonObj){
                                var varname = data.name + "." + k ;
                                $scope.deviceVariables[varname] = {
                                    value: jsonObj[k],
                                    name : data.name + "." + k,
                                    class: "col-md-3 col-sm-6"
                                } ;
                            }
                        } catch(e){
                            // isn't json
                            $scope.deviceVariables[data.name] = {
                                value : data.result,
                                name : data.name,
                                class: "col-xs-12"
                            } ;
                            console.log("changed",data) ;
                        }
                    },0) ;
                }
            }) ;

            utils.ajaxindicatorstart("Fetching device infos from Particle cloud...") ;
            spark.login({accessToken:$scope.access_token},function(err){
                if (err) {
                    utils.ajaxindicatorstop() ;
                    console.log(err) ;
                    alert(err) ;
                    return ;
                }

                $scope.rotilio.listDevices(function(err){
                    if (err) console.log(err) ;
                    utils.ajaxindicatorstop() ;
                });

            }) ;

            $scope.callFunction = function(device,funcName){
                if (!device.functionResponses) device.functionResponses = {} ;
                device.functionResponses[funcName] = "Calling..." ;
                var funParamValue = device.functionCalls[funcName] ;
                device.callFunction(funcName, funParamValue, function(err, data) {
                    if (err) {
                        console.log('An error occurred:', err);
                        device.functionResponses[funcName] = err ;
                    } else {
                        console.log('Function called succesfully:', data);
                        device.functionResponses[funcName] = data ;
                    }
                });
            }

            $scope.createFastUrl = function(device,funcName){
                // http://localhost:8000/rotilio/#/fast?access_token=a0fe08ad1a8fe9ca4ba1a102759ce2ea48b3597a&deviceid=23001d001447343339383037&functionname=message&args=setrelais:1000
                var funParamValue = device.functionCalls[funcName] ;
                var url = "./#/fast?access_token=:access_token&deviceid=:deviceId&functionname=:functionName&args=:args"
                    .replace(':deviceId', device.id)
                    .replace(":functionName", funcName)
                    .replace(":access_token", $scope.access_token )
                    .replace(":arg", encodeURIComponent(funParamValue));
                //$location.path(url) ;
                //window.location = url ;
                window.open(url,'_blank')
            }

            $scope.subscribeToEvents = function(device){
                device.eventlog = [] ;
                spark.getEventStream(false, device.id, function(data) {
                    console.log("Event ", data);
                    if (device.eventlog.length >= 10) device.eventlog.splice(device.eventlog.length-1,1) ;
                    $timeout(function(){
                        device.eventlog.splice(0, 0, data);
                    },0) ;
                });
            }

            $scope.filterDevices = function(value,index,array){
                if (!$scope.searchText) return true ;
                return (value.name.toLowerCase().indexOf($scope.searchText.toLowerCase()) > -1) ;
            }

            $scope.$on("$destroy", function(){
                delete($scope.rotilio) ;
            });

}]);