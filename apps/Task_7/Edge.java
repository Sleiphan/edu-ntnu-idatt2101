public class Edge {
  Edge next;
  Node to;

  public Edge(Node t, Edge n) {
    to = t;
    next = n;
  }
}
