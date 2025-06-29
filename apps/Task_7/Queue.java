public class Queue {
  private final Object[] tab;
  private int start = 0;
  private int end = 0;
  private int elements = 0;

  public Queue(int size) {
    tab = new Object[size];
  }

  public boolean isEmpty() {
    return elements == 0;
  }

  public boolean isFull() {
    return elements == tab.length;
  }

  public void enqueue(Object e) {
    if (isFull()) {
      return;
    }
    tab[end] = e;
    end = (end + 1) % tab.length;
    ++elements;
  }

  public Object nextInQueue() {
    if (!isEmpty()) {
      Object e = tab[start];
      start = (start + 1) % tab.length;
      --elements;
      return e;
    } else {
      return null;
    }
  }

  public Object checkQueue() {
    if (!isEmpty()) {
      return tab[start];
    }
    return null;
  }

  public void clear() {
    start = 0;
    end = 0;
    elements = 0;
  }
}
