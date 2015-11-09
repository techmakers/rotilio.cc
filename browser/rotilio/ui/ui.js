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

            var myParticleAdapter = new ParticleAdapter($http) ;

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

            $scope.saveElementValues = function(){
                $scope.uiElementsValue = {} ;
                $scope.uiElements.forEach(function(uiElementRow){
                    uiElementRow.forEach(function(uiElement){
                        var varName = uiElement.n ;
                        $scope.uiElementsValue[varName] = uiElement.value ;
                    }) ;
                }) ;
            };

            myParticleAdapter.getDeviceInfo(
                $scope.device,
                $scope.access_token,
                function(response){
                    $scope.deviceInfo = {} ;
                    console.log("deviceInfo",response);
                    if (response.status == 200){
                        $scope.deviceInfo = response.data ;
                    }
                },
                function(response){
                    alert(JSON.stringify(response));
                }
            )

            $scope.$on("$destroy", function(){

            });



            // preparing container for UI Elements
            var uiElementsItem = localStorage.getItem("uiElements") ;
            if (uiElementsItem){
                $scope.uiElements = JSON.parse(uiElementsItem) || [] ;
            } else {
                $scope.uiElements = [] ;
            }



            $scope.eventSourceTriggers = {
                "uiConfig" : function(e){
                    //console.log('uiConfig',e) ;
                    // data: "{"data":"{0:[{'n':'status.temperature','l':'Temperature'}, {'n':'status.temperaturesetpoint','min':-10,'max':30,'l':'Set temp:','t':'slider'}]}","ttl":"60","published_at":"2015-11-08T20:54:01.811Z","coreid":"30001c000647343232363230"}"
                    var data = JSON.parse(e.data).data.replace(/'/g,'"') ;
                    console.log("data for uiConfig",data) ;
                    var dataObj = JSON.parse(data);
                    // retaining actual element value
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
                        if ($scope.uiElementsValue[uiElement.n]){
                            uiElement.value = $scope.uiElementsValue[uiElement.n] ;
                        }
                    });
                }
            } ;

            // we will receive ui configuration elements via "publish" from the device itself
            $scope.eventSource = myParticleAdapter.subscribeToEvents($scope.device,$scope.access_token,$scope.eventSourceTriggers) ;


            // readingVariables
            $scope.readingStatusVariableTentativeCount = 0 ;
            $scope.readStatusVariable = function(cb){
                if (!cb) cb = function(){} ;
                $scope.readingStatusVariableTentativeCount++ ;
                if ($scope.readingStatusVariableTentativeCount > 10){
                    $scope.readingStatusVariableTentativeCount = 0 ;
                    console.log("too much $scope.readingStatusVariableTentativeCount") ;
                }
                myParticleAdapter.readVariable(
                    $scope.device,
                    $scope.access_token,
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
            } ;


            // interface
            $scope.resetChanged = function(){
                $scope.uiElements.forEach(function(uiElementRow){
                    uiElementRow.forEach(function(uiElement){
                        uiElement.changed = false ;
                    }) ;
                }) ;
            };

            $scope.updateValue = function(uiElement){
                var functionArgs = uiElement.n + ":" + uiElement.value ;
                var functionName = 'message' ;
                myParticleAdapter.callFunction(
                    $scope.device,
                    $scope.accessToken,
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
            } ;

            $scope.buttonClick = function(uiElement){
                var functionArgs = uiElement.m ;
                var functionName = 'message' ;
                myParticleAdapter.callFunction(
                    $scope.device,
                    $scope.access_token,
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
            } ;

            $scope.reload = function(){
                localStorage.removeItem("uiElements") ;
                location.reload(true);
            }

            if ($scope.uiElements.length > 0){
                utils.ajaxindicatorstop() ;
                $scope.readStatusVariable(function(){
                    $scope.saveElementValues() ;
                    $scope.refreshUiConfig() ;
                }) ;
            } else {
                $scope.refreshUiConfig() ;
            }

            // requesting UI Configuration
            $scope.refreshUiConfig = function(){
                myParticleAdapter.callFunction(
                    $scope.device,
                    $scope.access_token,
                    'message',
                    'getuiconfig:now',
                    function(successResponse){
                        console.log('ok', successResponse) ;
                        if (successResponse.status == 200){
                            // at this time photon already sent all config intems in $scope.uiElements ;
                            localStorage.setItem("uiElements",JSON.stringify($scope.uiElements)) ;
                            $scope.readStatusVariable(function(){
                                utils.ajaxindicatorstop() ;
                            });
                        }

                    },
                    function(failureResponse){
                        console.log("nok", failureResponse) ;
                    }
                ) ;
            };


            $interval(function(){
                $scope.readStatusVariable() ;
            },10000) ;


}]);