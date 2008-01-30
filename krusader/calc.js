
function Calculator(ui)
{
  // Setup entry functions
  var display = ui.child('display');
  this.display = display;

  this.one = function() { display.intValue = display.intValue*10+1; }
  this.two = function() { display.intValue = display.intValue*10+2; }
  this.three = function() { display.intValue = display.intValue*10+3; }
  this.four = function() { display.intValue = display.intValue*10+4; }
  this.five = function() { display.intValue = display.intValue*10+5; }
  this.six = function() { display.intValue = display.intValue*10+6; }
  this.seven = function() { display.intValue = display.intValue*10+7; }
  this.eight = function() { display.intValue = display.intValue*10+8; }
  this.nine = function() { display.intValue = display.intValue*10+9; }
  this.zero = function() { display.intValue = display.intValue*10+0; }

  ui.connect( ui.child('one'), 'clicked()', this, 'one' );
  ui.connect( ui.child('two'), 'clicked()', this, 'two' );
  ui.connect( ui.child('three'), 'clicked()', this, 'three' );
  ui.connect( ui.child('four'), 'clicked()', this, 'four' );
  ui.connect( ui.child('five'), 'clicked()', this, 'five' );
  ui.connect( ui.child('six'), 'clicked()', this, 'six' );
  ui.connect( ui.child('seven'), 'clicked()', this, 'seven' );
  ui.connect( ui.child('eight'), 'clicked()', this, 'eight' );
  ui.connect( ui.child('nine'), 'clicked()', this, 'nine' );
  ui.connect( ui.child('zero'), 'clicked()', this, 'zero' );

  this.val = 0;
  this.lastop = function() {}

  this.plus = function()
              {
                 this.val = display.intValue+this.val;
                 display.intValue = 0;
                 this.lastop=this.plus
              }

  this.minus = function()
               {
                  this.val = display.intValue-this.val;
                  display.intValue = 0;
                  this.lastop=this.minus;
               }


  ui.connect( ui.child('plus'), 'clicked()', this, 'plus' );
  ui.connect( ui.child('minus'), 'clicked()', this, 'minus' );

  this.equals = function() { this.lastop(); display.intValue = this.val; }
  this.clear = function() { this.lastop=function(){}; display.intValue = 0; this.val = 0; }

  ui.connect( ui.child('equals'), 'clicked()', this, 'equals' );
  ui.connect( ui.child('clear'), 'clicked()', this, 'clear' );
  
  this.setCap = function() { Krusader.setCaption( display.intValue ); }
  ui.connect( ui.child('setCap'), 'clicked()', this, 'setCap' );
}

var ui = Factory.loadui( scriptDir + 'calc.ui' );
var calc = new Calculator(ui);

ui.show();
