'use strict';

angular.module('myApp.home', ['ngRoute'])

.config(['$routeProvider', function($routeProvider) {
  $routeProvider.when('/home', {
    templateUrl: 'home/home.html',
    controller: 'HomeCtrl'
  });
}])

    .controller('HomeCtrl', ['$scope','$timeout','$interval','$location','$rootScope',
      function($scope,$timeout,$interval,$location,$rootScope) {

        function showLogin(){
          if ($rootScope.showedLogin) return ;
          $rootScope.showedLogin = true ;
          sparkLogin(function(data) { // loggin in
            console.log(data) ;
            $scope.access_token = data.access_token ;
            $rootScope.access_token = data.access_token ;
            $location.search("access_token", data.access_token);
            rotilio.listDevices();
          });
        }

        $scope.devices = [] ;
        var rotilio = new rotiliocc({
          refreshVariablePeriod : 0, // no refresh needed
          deviceAdded: function(device){
            $timeout(function(){
              $scope.devices.push(device) ;
            },0)
          }
        }) ;

        var savedState = $location.search();
        $scope.access_token = savedState.access_token ;
        $rootScope.access_token = $scope.access_token ;

        if (!$scope.access_token){
          showLogin() ;
        } else {
          spark.login({accessToken:$scope.access_token},function(err){
            if (err) {
              showLogin();
            }
            rotilio.listDevices();
          }) ;
        }

        $scope.filterDevices = function(value,index,array){
          if (!$scope.searchText) return true ;
          return (value.name.toLowerCase().indexOf($scope.searchText.toLowerCase()) > -1) ;
        }


      }]);