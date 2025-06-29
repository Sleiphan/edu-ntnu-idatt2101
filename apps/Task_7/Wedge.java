public class Wedge extends Edge {

  int capacity;
  Wedge reverse;
  public Wedge(Node t, Edge n, int w) {
    super(t, n);
    capacity = w;
    this.reverse = null;
  }
}
