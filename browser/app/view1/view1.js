'use strict';

angular.module('myApp.view1', ['ngRoute'])

.config(['$routeProvider', function($routeProvider) {
  $routeProvider.when('/view1', {
    templateUrl: 'view1/view1.html',
    controller: 'View1Ctrl'
  });
}])

.controller('View1Ctrl', ['$scope','$timeout','$interval','$location','$anchorScroll',
        function($scope,$timeout,$interval,$location,$anchorScroll) {

        $scope.devices = [] ;
        var rotilio = new rotiliocc({
            admittedDevices : ["Rotilio0001"],
            deviceAdded: function(device){
                $timeout(function(){
                   $scope.devices.push(device) ;
                },0)
            },
            variableChanged :function(data){
                $timeout(function(){
                    console.log("changed",data) ;
                },0) ;
            }
        }) ;

        var savedState = $location.search();
        $scope.access_token = savedState.access_token ;
        if (!$scope.access_token){
            sparkLogin(function(data) { // loggin in
                console.log(data) ;
                $scope.access_token = data.access_token ;
                $location.search("access_token", data.access_token);
                rotilio.listDevices();
            });
        } else {
            spark.login({accessToken:$scope.access_token},function(err){
                if (err) {
                    sparkLogin(function(data) { // loggin in
                        console.log(data) ;
                        $scope.access_token = data.access_token ;
                        $location.search("access_token", data.access_token);
                        rotilio.listDevices();
                    });
                }
                rotilio.listDevices();
            }) ;
        }

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

        $scope.subscribeToEvents = function(device){
            device.eventlog = [] ;
            spark.getEventStream(false, device.id, function(data) {
                console.log("Event ", data);
                if (device.eventlog.length >= 10) device.eventlog.splice(0,1) ;
                $timeout(function(){
                    device.eventlog.push(data) ;
                    $location.hash('bottom');
                    $anchorScroll();
                },0) ;
            });
        }

        $scope.filterDevices = function(value,index,array){
            if (!$scope.searchText) return true ;
            return (value.name.toLowerCase().indexOf($scope.searchText.toLowerCase()) > -1) ;
        }


}]);