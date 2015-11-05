'use strict';

angular.module('myApp.fast', ['ngRoute'])

    .config(['$routeProvider', function ($routeProvider) {
        $routeProvider.when('/fast', {
            templateUrl: 'fast/fast.html',
            controller: 'FastCtrl'
        });
    }])

    .controller('FastCtrl', ['$scope', '$timeout', '$interval', '$location', '$rootScope', '$http', 'utils',
        function ($scope, $timeout, $interval, $location, $rootScope, $http, utils) {

            // http://localhost:8000/rotilio/#/fast?access_token=a0fe08ad1a8fe9ca4ba1a102759ce2ea48b3597a&deviceid=23001d001447343339383037&functionname=message&args=setrelais%3a1000

            $scope.showSetup = false ;

            $scope.showResponse = false ;

            $scope.running = false ;
            $scope.success = false ;
            $scope.failure = false ;

            var savedState = $location.search();
            $scope.config = {
                title : savedState.title
            }

            if (!$scope.config.title) $scope.config.title = "My Rotilio Btn" ;

            window.document.title = $scope.config.title ;

            $scope.setTitle = function(){
                $location.search("title", $scope.config.title);
                window.document.title = $scope.config.title ;
            }

            $scope.goButtonClicked = function () {

                $scope.running = true ;
                $scope.success = false ;
                $scope.failure = false ;

                var savedState = $location.search();
                var device = savedState.deviceid;

                if (!device) {
                    $scope.response = "No device specified" ;
                    return ;
                }

                var access_token = savedState.access_token;
                if (!access_token){
                    $scope.response = "No access token specified" ;
                    return ;
                }


                var functionName = savedState.functionname;

                if (!functionName){
                    $scope.response = "No function name specified" ;
                    return ;
                }


                var functionArgs = savedState.args ;
                if (!functionArgs){
                    $scope.response = "No function args specified";
                    return ;
                }


                var url = "https://api.particle.io/v1/devices/:deviceId/:functionName?access_token=:access_token&args=:arg"
                    .replace(':deviceId', device)
                    .replace(":functionName", functionName)
                    .replace(":access_token", access_token)
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

                    }, function errorCallback(response) {
                        // called asynchronously if an error occurs
                        // or server returns response with an error status.
                        $scope.running = false ;
                        $scope.success = false ;
                        $scope.failure = true ;
                        $scope.response = response;
                    });
            }


        }]);