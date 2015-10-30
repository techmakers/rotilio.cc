function rotiliocc(options){

    // usage var myrotilio = new rotiliocc() ;

    var self = this ;

    this.options = options || {} ;
    this.options.refreshVariablePeriod = this.options.refreshVariablePeriod || 3000 ;
    this.options.deviceAdded = this.options.deviceAdded || function(){} ;
    this.options.variableChanged = this.options.variableChanged || function(){} ;

    this.devices = [] ;

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
                        //$timeout(function () { // needed to force AngularJS to update the view
                            var oldValue = device.variableValues[data.name] ;
                            device.variableValues[data.name] = data.result;
                            if (oldValue != data.result){
                                self.options.variableChanged(data)
                            }
                            device.updatedVars++ ;
                            if (device.updatedVars === device.variableCount) {
                                device.updatingVariables = false ;
                                device.lastUpdate = new Date() ;
                            } // if all variables responded we clear the device.updating flag
                        //}, 0);
                    }
                });
            }
        }
    }

    this.listDevices = function(cb){

        // example: ["rotilio0001","deopiipeoioei"] will list 2 devices,
        //          null : will list all devices

        self.devices = [] ; // initializing array for devices (you can manage more than one device with one account)
        spark.listDevices(function(err, devices) {
            if (err) return cb(err);
            var totDevices = devices.length ;
            var fetchedDevices = 0 ;
            setTimeout(function(){
                if (totDevices > fetchedDevices) return cb(new Error("Rotiliocc: timeout fetching all devices.")) ;
            },30000)
            devices.forEach(function(deviceDescriptor){
                var found = !self.options.admittedDevices ||
                    self.options.admittedDevices.indexOf(deviceDescriptor.id) > -1 ||
                    self.options.admittedDevices.indexOf(deviceDescriptor.name) > -1 ;
                if (!found){
                    fetchedDevices++ ;
                    return ;
                }
                spark.getDevice(deviceDescriptor.id, function(err, device) {
                    if (err) return alert(err) ;
                    fetchedDevices++ ;
                    self.devices.push(device) ;
                    self.options.deviceAdded(device) ;
                    if (fetchedDevices === totDevices) return cb() ;
                });
            }) ;
        });
    }

    this.updateAllDevices = function(){
        self.devices.forEach(function(device){
            updateDeviceVariables(device) ;
        }) ;
    }


    if (this.options.refreshVariablePeriod > 0){
        setInterval(this.updateAllDevices,0) ;
    }

}