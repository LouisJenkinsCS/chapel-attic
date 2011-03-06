use LevelBC_def;


//|\""""""""""""""""""""""""""""""""""""""""""|\
//| >    ZeroFluxDiffusionBC derived class    | >
//|/__________________________________________|/
class ZeroFluxDiffusionBC: LevelBC {
  
  def apply(q: LevelVariable, t: real) {
    apply_Homogeneous(q);
  }

  
  def apply_Homogeneous(q: LevelVariable) {
    
    for grid in level.grids {
    
      for ghost_domain in grid.ghost_multidomain {
        var loc   = grid.relativeLocation(ghost_domain);
        var shift = -1*loc;
        
        forall cell in ghost_domain do
          q(grid).value(cell) = q(grid).value(cell+shift);
      }
    
    } // end for grid in level.grids
  }

}
// /|""""""""""""""""""""""""""""""""""""""""""/|
//< |    ZeroFluxDiffusionBC derived class    < |
// \|__________________________________________\|