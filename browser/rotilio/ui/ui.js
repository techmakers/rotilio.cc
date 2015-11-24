'use strict';

angular.module('myApp.ui', ['ngRoute'])

.config(['$routeProvider', function($routeProvider) {
  $routeProvider.when('/ui', {
    templateUrl: 'ui/ui.html',
    controller: 'UiCtrl'
  });
}])

.controller('UiCtrl', ['$scope','$timeout','$interval','$location','$http','utils','$rootScope',
        function($scope,$timeout,$interval,$location,$http,utils,$rootScope) {

            var myParticleAdapter = new ParticleAdapter($http) ;

            utils.ajaxindicatorstart("Fetching device infos from Particle cloud...") ;

            $scope.running = false ;
            $scope.success = false ;
            $scope.failure = false ;

            var savedState = $location.search();
            $scope.device = savedState.deviceid;
            $scope.access_token = savedState.access_token;

            $rootScope.access_token = $scope.access_token ;

            $scope.uiElementsValue = {} ;

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
                "variableChanged":function(e){
                    console.log("variableChanged", e.data) ;
                    var data = JSON.parse(e.data).data ;
                    var components = data.split(":") ;
                    var varname = components[0] ;
                    components.splice(0,1) ;
                    var varvalue = components.join(":") ;
                    if (!isNaN(varvalue)) varvalue = varvalue*1 ;
                    $scope.uiElements.forEach(function(uiElementRow){
                        uiElementRow.forEach(function(uiElement){
                            if (uiElement.n == varname){
                                if (uiElement.value != varvalue){
                                    uiElement.changed = true ;
                                    uiElement.value = varvalue ;
                                    $timeout(function(){
                                        uiElement.changed = false ;
                                    },3000) ;
                                }
                            }
                        }) ;
                    }) ;
                },
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
                },
                "debugmsg": function(e){
                    $scope.debugmsg = e ;
                }
            } ;

            // we will receive ui configuration elements via "publish" from the device itself
            $scope.eventSource = myParticleAdapter.subscribeToEvents($scope.device,$scope.access_token,$scope.eventSourceTriggers) ;


            // reading status Variables
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
                        console.log("ok var status",okResponse) ;
                        try{
                            var data = JSON.parse(okResponse.data.result);
                        } catch (e){
                            return e ;
                        }
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

            // reading statitical variable

            $scope.stats = {} ;
            $scope.readingStatsVariableTentativeCount = 0 ;
            $scope.readStatsVariable = function(cb){
                if (!cb) cb = function(){} ;
                $scope.readingStatsVariableTentativeCount++ ;
                if ($scope.readingStatsVariableTentativeCount > 10){
                    $scope.readingStatsVariableTentativeCount = 0 ;
                    console.log("too much $scope.readingStatsVariableTentativeCount") ;
                }
                myParticleAdapter.readVariable(
                    $scope.device,
                    $scope.access_token,
                    "stats",
                    function(okResponse){
                        $scope.readingStatsVariableTentativeCount = 0 ;
                        console.log("ok var stats",okResponse) ;
                        var data = JSON.parse(okResponse.data.result);
                        for (var varName in data){
                            var value = data[varName] ;
                            if (!$scope.stats[varName]) $scope.stats[varName] = {} ;
                            if ($scope.stats[varName].value != data[varName]) $scope.stats[varName].changed = true ;
                            $scope.stats[varName].value = data[varName] ;
                        }
                        $timeout(function(){
                            $scope.resetStatsChanged() ;
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

            $scope.resetStatsChanged = function(){
                for (var key in $scope.stats){
                    $scope.stats[key].changed = false ;
                }
            };

            $scope.updateValue = function(uiElement){
                var functionArgs = uiElement.n + ":" + uiElement.value ;
                var functionName = 'message' ;
                myParticleAdapter.callFunction(
                    $scope.device,
                    $scope.access_token,
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
            };

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


            if ($scope.uiElements.length > 0){
                utils.ajaxindicatorstop() ;
                $scope.readStatusVariable(function(){
                    $scope.saveElementValues() ;
                    $scope.refreshUiConfig() ;
                }) ;
            } else {
                $scope.refreshUiConfig() ;
            }




            $interval(function(){
                $scope.readStatsVariable();
            },10000) ;

            $scope.readStatusVariable() ;
            $scope.readStatsVariable();
}]);