'use strict';

describe('myApp.home module', function() {

  beforeEach(module('myApp.home'));

  describe('home controller', function(){

    it('should ....', inject(function($controller) {
      //spec body
      var view2Ctrl = $controller('HomeCtrl');
      expect(view2Ctrl).toBeDefined();
    }));

  });
});