angular.module('myApp.services', [])
    .factory("utils",[function(){
    return {
        readArrayFromCSV : function(data, fieldNamesInFirstRow, rowSeparator, fieldSeparator){

            function isEnclosedInQuotes(text){
                if (!text) return false ;
                try{
                    var l = text.length ;
                    var start = text.substr(0,1) ;
                    var end = text.substr(l-1,1) ;
                    return (start == '"' && end == '"') ;
                } catch(e) {
                    return false ;
                }
            }

            function removeQuotes(text){
                if (text.replace) return text.replace('"','').replace('"','').trim() ;
                return text ;
            }

            var output = [] ;
            var fieldNames = [] ;
            var rows = data.split(rowSeparator || "\r") ;
            for (var i=0;i<rows.length;i++){
                var row = rows[i] ;
                var fieldValues = row.split(fieldSeparator || ";") ;
                if (i==0){
                    if (fieldNamesInFirstRow) {
                        for (var f=0;f<fieldValues.length;f++){
                            var v = fieldValues[f];
                            fieldNames.push(removeQuotes(v)) ;
                        }
                    }
                } else {
                    var record = {} ;
                    for (var ii=0;ii<fieldNames.length;ii++){
                        var fieldName = fieldNames[ii] ;
                        var fieldValue = fieldValues[ii] ;

                        // checking if number
                        if (isEnclosedInQuotes(fieldValue)){
                            record[fieldName] = removeQuotes(fieldValue) ;
                        } else {
                            var num = parseFloat(fieldValue) ;
                            if (!isNaN(num)){
                                record[fieldName] = num ;
                            } else {
                                record[fieldName] = fieldValue ;
                            }
                        }
                    }
                    output.push(record) ;
                }
            }

            return output ;
        },

        ajaxindicatorstart: function(text)
        {
            if(jQuery('body').find('#resultLoading').attr('id') != 'resultLoading'){
                jQuery('body').append('<div id="resultLoading" style="display:none"><div><img src="pics/ajax-loader.gif"><div>'+text+'</div></div><div class="bg"></div></div>');
            }

            jQuery('#resultLoading').css({
                'width':'100%',
                'height':'100%',
                'position':'fixed',
                'z-index':'10000000',
                'top':'0',
                'left':'0',
                'right':'0',
                'bottom':'0',
                'margin':'auto'
            });

            jQuery('#resultLoading .bg').css({
                'background':'#000000',
                'opacity':'0.7',
                'width':'100%',
                'height':'100%',
                'position':'absolute',
                'top':'0'
            });

            jQuery('#resultLoading>div:first').css({
                'width': '250px',
                'height':'75px',
                'text-align': 'center',
                'position': 'fixed',
                'top':'0',
                'left':'0',
                'right':'0',
                'bottom':'0',
                'margin':'auto',
                'font-size':'16px',
                'z-index':'10',
                'color':'#ffffff'

            });

            jQuery('#resultLoading .bg').height('100%');
            jQuery('#resultLoading').fadeIn(300);
            jQuery('body').css('cursor', 'wait');
        },

        ajaxindicatorstop: function()
        {
            jQuery('#resultLoading .bg').height('100%');
            jQuery('#resultLoading').fadeOut(300,function(){
                jQuery('#resultLoading').remove();
            });
            jQuery('body').css('cursor', 'default');
        }

    }
}]);
