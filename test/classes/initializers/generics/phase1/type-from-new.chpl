
record R {
  var x : int;
}

class C {
  type t;
  var v = new R();

  proc init() {
    t = new R();
    this.initDone();
    v = new R();
  }
}

proc main() {
  var c = new C();
  delete c;
}
