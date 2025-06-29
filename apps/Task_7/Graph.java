import java.io.BufferedReader;
import java.io.IOException;
import java.util.*;

public class Graph {
  int N, E;
  Node []node;

  public void newGraph(BufferedReader br) throws IOException {
    StringTokenizer st = new StringTokenizer(br.readLine());
    N = Integer.parseInt(st.nextToken());
    node = new Node[N];
    for (int i = 0; i < N; i++) {
      node[i] = new Node();
      node[i].nodeNumber = i;
    }
    E = Integer.parseInt(st.nextToken());
    for (int i = 0; i < E; i++) {
      st = new StringTokenizer(br.readLine());
      int from = Integer.parseInt(st.nextToken());
      int to = Integer.parseInt(st.nextToken());
      int weight = Integer.parseInt(st.nextToken());
      node[from].edge = new Wedge(node[to], node[from].edge, weight); // Maybe swap weight values
      node[to].edge = new Wedge(node[from], node[to].edge, 0);     // With this weight value
      ((Wedge) node[from].edge).reverse = (Wedge) node[to].edge;
      ((Wedge) node[to].edge).reverse = (Wedge) node[from].edge;
    }
  }

  public void initPrev(Node s) {
    for (int i = N; i-- > 0; ) {
      node[i].d = new Prev();
    }
    ((Prev) s.d).dist = 0;
  }

  public ArrayList<Wedge> bfs(Node source, Node sink) {
    initPrev(source);
    BFSResults[] results = new BFSResults[N];
    for (int i = 0; i < N; i++) {
      results[i] = new BFSResults();
      results[i].prevNode = -1;
    }
    Queue queue = new Queue(N - 1);
    queue.enqueue(source);
    boolean foundSink = false;
    while (!queue.isEmpty() && !foundSink) {
      Node n = (Node) queue.nextInQueue();

      for (Edge e = n.edge; e != null; e = e.next) {
        Prev p = (Prev) e.to.d;
        if (p.dist == p.infinity && ((Wedge) e).capacity > 0) {
          p.dist = ((Prev) n.d).dist + 1;
          p.prev = n;
          queue.enqueue(e.to);
          results[e.to.nodeNumber].dist = 0;
          results[e.to.nodeNumber].prevEdge = (Wedge) e;
          results[e.to.nodeNumber].prevNode = n.nodeNumber;

          if (e.to == sink) {
            foundSink = true;
            break;
          }
        }
      }
    }
    ArrayList<Wedge> path = new ArrayList<>();

    for (BFSResults n = results[sink.nodeNumber]; n.prevNode != -1; n = results[n.prevNode]) {
      path.add(n.prevEdge);
    }
    Collections.reverse(path);

    return path;
  }

  public FlowResult maxFlow(Node source, Node sink) {
    FlowResult result = new FlowResult();

    for (ArrayList<Wedge> path = bfs(source, sink); !path.isEmpty(); path = bfs(source, sink)){
      int bottle_neck = Integer.MAX_VALUE;

      for (Wedge e : path) {
        bottle_neck = Math.min(e.capacity, bottle_neck);
      }

      for (Wedge e : path) {
        e.capacity -= bottle_neck;
        e.reverse.capacity += bottle_neck;
      }

      result.increases.add(bottle_neck);
      result.paths.add(new int[path.size() + 1]);
      result.paths.get(result.paths.size() - 1)[0] = source.nodeNumber;
      for (int i = 0; i < path.size(); i++) {
        result.paths.get(result.paths.size() - 1)[i + 1] = path.get(i).to.nodeNumber;
      }
      result.maxFlow += bottle_neck;
    }

    return result;
  }

  public void printMaxFlow(FlowResult maxFlow, Node source, Node sink) {
    System.out.println("Maximum flow from " + source.nodeNumber + " to " + sink.nodeNumber + " with Edmond-Karp method");
    System.out.println("Increase : Increasing flow path");

    for (int i = 0; i < maxFlow.increases.size(); i++) {
      System.out.print(maxFlow.increases.get(i) + "        | ");
      for (int p = 0; p < maxFlow.paths.get(i).length; p++) {
        System.out.print((maxFlow.paths.get(i)[p]) + " ");
        //System.out.print(Arrays.toString(maxFlow.paths.get(p)));
      }
      System.out.println();
    }
    System.out.println("Maximal flow is " + maxFlow.maxFlow);
  }
}
