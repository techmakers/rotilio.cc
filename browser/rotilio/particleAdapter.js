var ParticleAdapter = function(httpAdapter){

    /*
    *
    * httpAdapter: $http of angular JS
    *
    *
    * */

    return {

        getDeviceList : function(access_token,onSuccessCb,onErrorCb){
            var url = "https://api.particle.io/v1/devices/?access_token=:access_token"
                .replace(":access_token", access_token);

            var req = {method: 'GET', url: url} ;
            httpAdapter(req).then(onSuccessCb, onErrorCb);
        },

        getDeviceInfo : function(device, access_token, onSuccessCb,onErrorCb){
            // GET /v1/devices/:deviceId
            var url = "https://api.particle.io/v1/devices/:deviceId/?access_token=:access_token"
                .replace(':deviceId', device)
                .replace(":access_token", access_token);

            var req = {method: 'GET', url: url} ;
            httpAdapter(req).then(onSuccessCb, onErrorCb);
        },

        readVariable : function(device, access_token, variableName, onSuccessCb, onErrorCb){
            // GET /v1/devices/0123456789abcdef01234567/temperature
            var url = "https://api.particle.io/v1/devices/:deviceId/:variablename?access_token=:access_token"
                .replace(':deviceId', device)
                .replace(":variablename", variableName)
                .replace(":access_token", access_token);

            var req = {
                method: 'GET',
                url: url
            }

            httpAdapter(req).then(onSuccessCb, onErrorCb);
        },

        callFunction : function(device, access_token, functionName, functionArgs,onSuccessCb,onErrorCb) {

            var url = "https://api.particle.io/v1/devices/:deviceId/:functionName?access_token=:access_token&args=:arg"
                .replace(':deviceId', device)
                .replace(":functionName", functionName)
                .replace(":access_token", access_token)
                .replace(":arg", encodeURIComponent(functionArgs));

            var req = {
                method: 'POST',
                url: url,
                data:{
                    args:functionArgs
                }
            }

            httpAdapter(req).then(onSuccessCb, onErrorCb);
        },

        subscribeToEvents : function(device, access_token, eventListeners){

            // GET /v1/devices/{DEVICE_ID}/events
            if (device) {
                var eventSourceUrl = "https://api.spark.io/v1/devices/" + device + "/events/?access_token=" + access_token ;
            } else {
                var eventSourceUrl = "https://api.spark.io/v1/events/?access_token=" + access_token ;
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
    }
}