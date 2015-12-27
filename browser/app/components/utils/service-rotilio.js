angular.module('myApp.services', [])
    .factory("rotilio",[function(){

        rotilioAdapter = new rotiliocc({
            refreshVariablePeriod : 0
        }) ;

        return {
            rotilio: rotilioAdapter
        }
    }]);
