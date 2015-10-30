'use strict';

angular.module('myApp.view2', ['ngRoute'])

.config(['$routeProvider', function($routeProvider) {
  $routeProvider.when('/view2', {
    templateUrl: 'view2/view2.html',
    controller: 'View2Ctrl'
  });
}])

    .controller('View2Ctrl', ['$scope','$timeout','$interval','$location','$anchorScroll',
      function($scope,$timeout,$interval,$location,$anchorScroll) {

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

        $scope.filterDevices = function(value,index,array){
          if (!$scope.searchText) return true ;
          return (value.name.toLowerCase().indexOf($scope.searchText.toLowerCase()) > -1) ;
        }


      }]);