//===> Description ===>
//
// Basic class and method definitions for a rectangular grid of
// generic dimension (though it must be a compile-time constant.)
//
// This version is based on an underlying mesh of width dx/2, which
// contains all cell centers, interface centers, and vertices.  As
// a result, the domain of cell centers is strided by 2, corresponding
// to jumps of dx.
//
//<=== Description <===

use SpaceDimension;
use LanguageExtensions;
use MultiDomain_def;



//-----------------------------------------------------------
// Define an enum type to describe the relative position of
// an index to a range.  Tuples of these will describe how
// a multi-dimensional index (and potentially a full domain)
// is positioned relative to a full grid.
//-----------------------------------------------------------

enum  loc1d {below=-2, low=-1, inner=0, high=1, above=2};




//|\"""""""""""""""""""|\
//| >    Grid class    | >
//|/___________________|/

class Grid {
  
  const x_low, x_high:          dimension*real;
  const i_low, i_high:          dimension*int;
  const n_cells, n_ghost_cells: dimension*int;

  const dx: dimension*real;
          
  const extended_cells: domain(dimension, stridable=true);
  const cells:          subdomain(extended_cells);
  
  const ghost_multidomain: MultiDomain(dimension, stridable=true);


  //|\''''''''''''''|\
  //| >    clear    | >
  //|/..............|/

  def clear () {
    delete ghost_multidomain;
  }
  // /|''''''''''''''/|
  //< |    clear    < |
  // \|..............\|


  //|\''''''''''''''''''''|\
  //| >    Constructor    | >
  //|/....................|/
  
  def Grid(
    x_low:         dimension*real,
    x_high:        dimension*real,
    i_low:         dimension*int,
    n_cells:       dimension*int,
    n_ghost_cells: dimension*int)
  {

    //==== Assign inputs to fields ====
    this.x_low         = x_low;
    this.x_high        = x_high;
    this.i_low         = i_low;
    this.n_cells       = n_cells;
    this.n_ghost_cells = n_ghost_cells;

    //==== Sanity check ====
    sanityChecks();

    //==== dx ====
    dx = (x_high - x_low) / n_cells;

    //==== i_high ====
    for d in dimensions do
      i_high(d) = i_low(d) + 2*n_cells(d);

    //==== Physical cells ====
    var ranges: dimension*range(stridable = true);
    for d in dimensions do ranges(d) = (i_low(d)+1 .. by 2) #n_cells(d);
    cells = ranges;


    //==== Extended cells (includes ghost cells) ====
    //---------------------------------------------------------------
    // The 'expand' method of an arithmetic domain extends both its
    // lower and upper bounds by the input.  In this case, the input
    // must be multiplied by 2 because a cell is 2 indices wide.
    //---------------------------------------------------------------
    extended_cells = cells.expand(2*n_ghost_cells);


    //===> Ghost cells ===>
    //----------------------------------------------------------------
    // Blocks of ghost cells are placed in ghost_multidomain, grouped
    // by their position relative to the grid.  For example, in 2d,
    // there are 8 blocks:
    //   (lower,lower), (lower,inner), (lower,upper)
    //   (inner,lower), (inner,upper),
    //   (upper,lower), (upper,inner), (upper,upper)
    //----------------------------------------------------------------

    ghost_multidomain = new MultiDomain(dimension, stridable=true);

    var inner_location: dimension*int;
    for d in dimensions do inner_location(d) = loc1d.inner;

    var ghost_domain: domain(dimension, stridable=true);
    for loc in (loc1d.below .. loc1d.above by 2)**dimension {
      if loc != inner_location {
        for d in dimensions do
          ranges(d) = if loc(d) == loc1d.below then 
                        (extended_cells.low(d).. by 2) #n_ghost_cells(d)
                      else if loc(d) == loc1d.inner then
                        cells.dim(d)
                      else
                        (..extended_cells.high(d) by 2) #n_ghost_cells(d);
        
        ghost_domain = ranges;
        ghost_multidomain.add(ghost_domain);
      }
    }
    //<=== Ghost cells <===

  }
  // /|''''''''''''''''''''/|
  //< |    Constructor    < |
  // \|....................\|



  //|\''''''''''''''''''''''''''|\
  //| >   sanityChecks method   | >
  //|/..........................|/

  //--------------------------------------------------------------
  // Performs some basic sanity checks on the constructor inputs.
  //--------------------------------------------------------------

  def sanityChecks () {
    var d_string: string;
    for d in dimensions do {
      d_string = format("%i", d);

      assert(x_low(d) < x_high(d),
	     "error: Grid: x_low(" + d_string + ") must be strictly less than x_high(" + d_string + ").");

      assert(n_cells(d) > 0,
             "error: Grid: n_cells(" + d_string + ") must be positive.");

      assert(n_ghost_cells(d) > 0,
	     "error: Grid: n_ghost_cells(" + d_string + ") must be positive.");
    }
  }
  // /|'''''''''''''''''''''''''''/|
  //< |   sanityChecks method    < |
  // \|...........................\|



  //|\''''''''''''''''''''''''''''''''|\
  //| >    relativeLocation method    | >
  //|/................................|/
  
  def relativeLocation(idx: dimension*int) {
    var loc: dimension*int;

    for d in dimensions {
           if idx(d) <  i_low(d)  then loc(d) = loc1d.below;
      else if idx(d) == i_low(d)  then loc(d) = loc1d.low;
      else if idx(d) <  i_high(d) then loc(d) = loc1d.inner;
      else if idx(d) == i_high(d) then loc(d) = loc1d.high;
      else                             loc(d) = loc1d.above;
    }

    return loc;
  }

  def relativeLocation(D: domain(dimension, stridable=true)){
    var loc_low  = relativeLocation(D.low);
    var loc_high = relativeLocation(D.high);

    if loc_low == loc_high then
      return loc_low;
    else
      halt("error: Grid.locateDomain\n:" +
	   "Provided domain spans multiple location categories.");
  }
  // /|''''''''''''''''''''''''''''''''/|
  //< |    relativeLocation method    < |
  // \|................................\|



  //|\'''''''''''''''''''''''''|\
  //| >    writeThis method    | >
  //|/.........................|/
  
  //-----------------------------------------------------------
  // Defines the output of the intrinsic 'write' and 'writeln' 
  // procedures, so that write(Grid) will produce something
  // sensible.  Mainly for testing and debugging.
  //-----------------------------------------------------------
  
  def writeThis (w: Writer) {
    writeln("x_low: ", x_low, ",  x_high: ", x_high);
    write("i_low: ", i_low, ",  i_high: ", i_high);
  }
  // /|'''''''''''''''''''''''''/|
  //< |    writeThis method    < |
  // \|.........................\|

}
// /|"""""""""""""""""""/|
//< |    Grid class    < |
// \|___________________\|







//|\"""""""""""""""""""""""""""|\
//| >    Grid.xValue method    | >
//|/___________________________|/

//-------------------------------------------------------------
// Converts an index to a coordinate tuple.  This is primarily
// used when evaluating an analyitical function on the Grid,
// as when setting up in initial condition.
//-------------------------------------------------------------

def Grid.xValue (point_index: dimension*int) {

  var coord: dimension*real;

  if dimension == 1 then {
    coord(1) = x_low(1) + (point_index(1) - i_low(1)) * dx(1)/2.0;
  }
  else {
    forall d in dimensions do
      coord(d) = x_low(d) + (point_index(d) - i_low(d)) * dx(d)/2.0;
  }

  return coord;
}
// /|"""""""""""""""""""""""""""/|
//< |    Grid.xValue method    < |
// \|___________________________\|






//|\"""""""""""""""""""""""""|\
//| >    readGrid routine    | >
//|/_________________________|/

//---------------------------------------------------------------
// Constructs a grid from an input file.  Used when implementing
// single-grid problems, but not for AMR.
//---------------------------------------------------------------

def readGrid(file_name: string) {

  var input_file = new file(file_name, FileAccessMode.read);
  input_file.open();

  var dim_in: int;
  input_file.readln(dim_in);
  assert(dim_in == dimension, 
         "error: dimension of space.txt must equal " + format("%i",dimension));
  input_file.readln(); // empty line

  var x_low, x_high:                 dimension*real;
  var n_cells, n_ghost_cells: dimension*int;

  input_file.readln( (...x_low) );
  input_file.readln( (...x_high) );
  input_file.readln( (...n_cells) );
  input_file.readln( (...n_ghost_cells) );

  input_file.close();

  //==== Set low indices ====
  //-------------------------------------------------------------
  // This routine should only be used for a single-grid problem,
  // so assume lower indicies are all zero.
  //-------------------------------------------------------------
  var i_low: dimension*int;


  return new Grid(x_low, x_high, i_low, n_cells, n_ghost_cells);

}
// /|"""""""""""""""""""""""""/|
//< |    readGrid routine    < |
// \|_________________________\|




//|\"""""""""""""""""""""""""""""""|\
//| >    setOutputTimes routine    | >
//|/_______________________________|/

def setOutputTimes (file_name: string) {

  var input_file = new file(file_name, FileAccessMode.read);
  input_file.open();

  var initial_time, final_time: real;
  var n_output: int;

  input_file.readln(initial_time);
  input_file.readln(final_time);
  input_file.readln(n_output);
  input_file.close();

  var output_times: [0..n_output] real;
  var dt_output:    real = (final_time - initial_time) / n_output;
  
  for i in [0..n_output] do
    output_times(i) = initial_time + i*dt_output;

  return output_times;

}
// /|"""""""""""""""""""""""""""""""/|
//< |    setOutputTimes routine    < |
// \|_______________________________\|












// def main {
// 
//   var x_low = (0.0,1.0);
//   var x_high = (2.0,3.0);
//   var i_low = (0,0);
//   var n_cells = (20,40);
//   var n_ghost_cells = (2,2);
// 
//   var grid = new Grid(x_low, x_high, i_low, n_cells, n_ghost_cells);
// 
//   writeln(grid);
//   writeln("grid.cells = ", grid.cells);
//   writeln("grid.extended_cells = ", grid.extended_cells);
// 
// 
// }