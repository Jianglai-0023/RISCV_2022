#include"sti.hpp"

const int QUEUE_SIZE = 32;

template<class K>
class queue {//循环队列，不可扩容，支持下标访问，编号重复
public:
    struct node {
        K value;
        node(){}
        node(const K &value):value(value){}
    };
    node *a;
    int num = 0;
    int front = 0, rear = 0;
    int maxSize;
public:
    explicit queue(int size = QUEUE_SIZE) {
        a = new node[size];
        maxSize = size;
    }

    ~queue() {
        delete[] a;
    };

    node pop() {
        front = (front + 1) % maxSize;
        return a[front];
    }

    node top() {
         return a[(front+1)%maxSize];
    }

    int in(K value) {
        rear = (rear + 1) % maxSize;
        a[rear] = node(value);
        return rear;
    }

    bool isfull() {
        return ((rear + 1) % maxSize == front);
    }

}

int main() {

}