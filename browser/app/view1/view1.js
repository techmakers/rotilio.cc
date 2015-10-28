'use strict';

angular.module('myApp.view1', ['ngRoute'])

.config(['$routeProvider', function($routeProvider) {
  $routeProvider.when('/view1', {
    templateUrl: 'view1/view1.html',
    controller: 'View1Ctrl'
  });
}])

.controller('View1Ctrl', ['$scope','$timeout','$interval','$location',function($scope,$timeout,$interval,$location) {

        var savedState = $location.search();
        $scope.access_token = savedState.access_token ;
        if (!$scope.access_token){
            sparkLogin(function(data) { // loggin in
                console.log(data) ;
                $scope.access_token = data.access_token ;
                $location.search("access_token", data.access_token);
                listAllDevices();
            });
        } else {
            spark.login({accessToken:$scope.access_token},function(err){
                if (err) {
                    sparkLogin(function(data) { // loggin in
                        console.log(data) ;
                        $scope.access_token = data.access_token ;
                        $location.search("access_token", data.access_token);
                        listAllDevices();
                    });
                }
                listAllDevices();
            }) ;
        }

        function prepareDeviceForUpdate(device) {
            device.timeout = false ;
            device.updatingVariables = true;
            device.updatingFrom = new Date().getTime();
            if (!device.variableValues) device.variableValues = {};
            device.variableCount = 0;
            device.updatedVars = 0;
        }

        function deviceTimedOut(device){
            var now = new Date().getTime() ;
            var delta = now - device.updatingFrom ;
            if (delta > 30000){   // 30 seconds timeout
                device.updatingVariables = false ;
                console.log("Device: '" + device.name + "' timed out !!   Retrying") ;
                device.timeout = true ;
                return true
            } else {
                return false ;   // no time out, still working
            }
        }

        function updateDeviceVariables(device) {
            if (device.variables) {
                if (device.updatingVariables && !deviceTimedOut(device)){
                    return ;
                }
                prepareDeviceForUpdate(device);
                for (var variable in device.variables) {
                    device.variableCount++ ;
                    device.getVariable(variable, function (err, data) {
                        if (err) {
                            console.log('An error occurred while getting attrs:', err);
                        } else {
                            $timeout(function () { // needed to force AngularJS to update the view
                                device.variableValues[data.name] = data.result;
                                device.updatedVars++ ;
                                if (device.updatedVars === device.variableCount) {
                                    device.updatingVariables = false ;
                                    device.lastUpdate = new Date() ;
                                } // if all variables responded we clear the device.updating flag
                            }, 0);
                        }
                    });
                }
            }
        }

        function listAllDevices(){
            $scope.devices = [] ; // initializing array for devices (you can manage more than one device with one account)
            spark.listDevices(function(err, devices) {
                if (err) return console.log(err) ;
                devices.forEach(function(deviceDescriptor){
                    spark.getDevice(deviceDescriptor.id, function(err, device) {
                        if (err) return alert(err) ;
                        $scope.devices.push(device) ;
                    });
                })

            });
        }

        function updateAllDevices(){
            $scope.devices.forEach(function(device){
                updateDeviceVariables(device) ;
            }) ;
        }

        $interval(function(){
            updateAllDevices();
        },3000) ;

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

        $scope.sendSignalToDevice = function(device){
            if (device.sendSignalCommand){
                device.signal(function(err, data) {
                    if (err) {
                        console.log('Error sending a signal to the device:', err);
                    } else {
                        console.log('Device is shouting rainbows:', data);
                    }
                });
            } else {
                device.stopSignal(function(err, data) {
                    if (err) {
                        console.log('Error sending stop signal to the device:', err);
                    } else {
                        console.log('The LED is back to regular colors:', data);
                    }
                });
            }

        }

        $scope.filterDevices = function(value,index,array){
            if (!$scope.searchText) return true ;
            return (value.name.toLowerCase().indexOf($scope.searchText.toLowerCase()) > -1) ;
        }
}]);