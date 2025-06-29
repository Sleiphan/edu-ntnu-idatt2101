import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class Main {

  public static void main(String[] args) throws IOException {
    BufferedReader flowGraph1 = new BufferedReader(new FileReader("flytgraf1"));
    BufferedReader flowGraph2 = new BufferedReader(new FileReader("flytgraf2"));
    BufferedReader flowGraph3 = new BufferedReader(new FileReader("flytgraf3"));
    BufferedReader flowGraph4 = new BufferedReader(new FileReader("flytgraf4"));
    BufferedReader flowGraph5 = new BufferedReader(new FileReader("flytgraf5"));

    Graph g = new Graph();
    g.newGraph(flowGraph4);

    Node source = g.node[0];
    Node sink = g.node[7];

    g.printMaxFlow(g.maxFlow(source, sink), source, sink);
  }
}