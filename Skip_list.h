#include <iostream>
#include <random>
#include <time.h>
#include <memory>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <atomic>

using namespace std;

/* Node init */

template <class T>
struct Node{

    T key;
    Node<T> **forward_level;

    size_t level_;

    Node(T k, size_t level);
    ~Node();
    void unlink();
};

template <class T>
Node<T>::Node(T k, size_t level)
{
    key = k;
    level_ = level;
    forward_level = new Node<T>*[level];
    unlink();
}

template <class T>
Node<T>::~Node()
{
    delete []forward_level;
}

template <class T>
void Node<T>::unlink()
{
    for (int i = 0; i < level_; i++)
        forward_level [i] = 0;
    
}

/* Node end */

/* Skip list init */

template <class T>
class Skip_list {

    size_t max_level_;

    Node<T>* init;

    std::atomic<size_t> element_count_;

    size_t  randomlevel();
    Node<T>* search_ptr(T k);
    std::vector<Node<T>*> get_previous_ptr(T k);

    mutable std::shared_mutex m;

    public:

        Skip_list(size_t level);

        bool Search(T k);
        void Insert(T k);
        Node<T>* Delete(T k);

        void Printer();
        size_t size();
};

template <class T>
Skip_list<T>::Skip_list(size_t level): max_level_(level), element_count_(0)
{
    T k;
    init = new Node<T>(k, level);
}


template <class T>
Node<T>* Skip_list<T>::search_ptr(T k)
{
    std::shared_lock<std::shared_mutex> sharedLock(m);
    Node<T>* prev = this->init;
    for (ssize_t i = this->max_level_ - 1; i >= 0; i--)
    {
        while (prev->forward_level [i] != nullptr && prev->forward_level [i]->key < k) 
            prev = prev->forward_level [i];
        
        if (prev->forward_level [i] != nullptr && prev->forward_level [i]->key == k)
            return prev;
    }

    return nullptr;
}

template <class T>
std::vector<Node<T>*> Skip_list<T>::get_previous_ptr(T k)
{

    std::vector<Node<T>*> prev(this->max_level_, this->init);
    Node<T>* curr = this->init;
    for (ssize_t i = this->max_level_ - 1; i >= 0; i--) {
        while (curr->forward_level [i] != nullptr && curr->forward_level [i]->key < k) {
            curr = curr->forward_level [i];
        }
        prev[i] = curr;
    }
    return prev;

}

template <class T>
bool Skip_list<T>::Search(T k)
{
    return search_ptr(k);
}

template <class T>
void Skip_list<T>::Insert(T k)
{
    std::unique_lock<std::shared_mutex> uniqueLock(m);

    Node<T>* node = new Node<T>(k, randomlevel());

    auto previous_ptrs = get_previous_ptr(k);

    for (ssize_t i = node->level_ - 1; i >= 0; --i)
    {
        node->forward_level [i] = previous_ptrs[i]->forward_level [i];
        previous_ptrs[i]->forward_level [i] = node;
    }

    ++element_count_;
}

template <class T>
size_t Skip_list<T>::size()
{
    return this->element_count_;
}

template <class T>
Node<T>* Skip_list<T>::Delete(T k)
{
    std::unique_lock<std::shared_mutex> uniqueLock(m);
    auto update = get_previous_ptr(k);


    if (update[0]->forward_level [0] == nullptr || update[0]->forward_level [0]->key != k)
        return 0;

    auto return_ptr = update[0]->forward_level [0];

    for (ssize_t i = 0; i < update.size(); i++)
    {
        if (update[i]->forward_level [i] != return_ptr)
            break;
        update[i]->forward_level [i] = return_ptr->forward_level [i];
    }

    element_count_--;

    return return_ptr;
}

template <class T>
size_t Skip_list<T>::randomlevel()
{

     int level = 1;
    
    float number = (float)(rand()) / (float)(RAND_MAX);

    while(number <= 0.5)
    {
        number = (float)(rand()) / (float)(RAND_MAX);
        level++;
    }

    return level <= max_level_? level : max_level_;
}

template <class T>
void Skip_list<T>::Printer()
{

    std::shared_lock<std::shared_mutex> sharedLock(m);

    for (ssize_t i = this->max_level_ - 1; i >= 0; --i)
    {
        cout << "l["<<  i + 1 << "] ";
        Node<T> *node = this->init->forward_level [0];
        
        while (node)
        {
            if(node->level_ - 1 >= i)
                cout << "-> " << node->key << " ";
            else
                cout << "-----";

            node = node->forward_level [0];
        }
        cout << "-> Null" <<endl;
    }
    cout << endl;
}

/* Skip list end */