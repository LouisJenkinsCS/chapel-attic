class node {
  type element_type = int;
  var element : element_type;
}

var n = new node();

n.element = 12;

writeln(n.element);
