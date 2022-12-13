#include <iostream>
#include <climits>
#include <cassert>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iomanip>

#include "Skip_list.h"

using namespace std;

void test_single_thread(int element_count, size_t list_level = 20) {
    Skip_list<int> list(list_level);
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < element_count; ++i) 
    {
        list.Insert(i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> insert_duration = end - start;

    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < element_count; ++i) 
    {
        list.Search(i);
    }
    
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> search_duration = end - start;
    

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < element_count; ++i) 
    {
        list.Delete(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> delete_duration = end - start;
    
    cout << "Single-thread\n";
    cout << "Insert duration: " << fixed << setprecision(10) << insert_duration.count() << "s" << endl;
    cout << "search duration: " << fixed << setprecision(10) << search_duration.count() << "s" << endl;
    cout << "erase  duration: " << fixed << setprecision(10) << delete_duration.count() <<  "s" << endl;
}

void insert_task(Skip_list<int>& Skip_list, size_t patch_size, size_t base) {
    for (int i = base; i < patch_size + base; ++i) {
        Skip_list.Insert(i);
    }
}

void search_task(Skip_list<int>& Skip_list, size_t patch_size, size_t base) {
    for (int i = base; i < patch_size + base; ++i) {
        assert(Skip_list.Search(i));
    }
}

void Delete_task(Skip_list<int>& Skip_list, size_t patch_size, size_t base) {
    for (int i = base; i < patch_size + base; ++i) {
        Skip_list.Delete(i);
    }
}

void test_multi_thread(size_t element_count, size_t list_level = 20,  size_t thread_count = std::thread::hardware_concurrency()) {
    Skip_list<int> Skip_list(list_level);

    /*thread information */
    auto every_thread_size = element_count / thread_count;
    auto total_size = every_thread_size * thread_count;

    /* insert 0->element count to skip list */
    vector<thread> insert_threads; insert_threads.reserve(thread_count);
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < thread_count; ++i) {
        insert_threads.emplace_back(insert_task, std::ref(Skip_list), every_thread_size, i * every_thread_size);
    }
    for_each(insert_threads.begin(), insert_threads.end(), [] (auto& t) {
        t.join();
    });
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> insert_duration = end - start;
    assert(Skip_list.size() == total_size);
    
    vector<thread> search_treads; search_treads.reserve(thread_count);
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < thread_count; ++i) {
        search_treads.emplace_back(search_task, std::ref(Skip_list), every_thread_size, i * every_thread_size);
    }

    for_each(search_treads.begin(), search_treads.end(), [] (auto& t) {
        t.join();
    });

    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> search_duration = end - start;
    assert(Skip_list.size() == total_size);

    
    vector<thread> erase_threads; erase_threads.reserve(thread_count);
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < thread_count; ++i) {
        erase_threads.emplace_back(Delete_task, std::ref(Skip_list), every_thread_size, i * every_thread_size);
    }

    for_each(erase_threads.begin(), erase_threads.end(), [] (auto& t) {
        t.join();
    });

    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> delete_duration = end - start;
    assert(Skip_list.size() == 0);

    cout << "Multi-thread:\n";
    cout << "Insert duration: " << fixed << setprecision(10) << insert_duration.count() << "s" << endl;
    cout << "search duration: " << fixed << setprecision(10) << search_duration.count() << "s" << endl;
    cout << "erase  duration: " << fixed << setprecision(10) << delete_duration.count() <<  "s" << endl;

}

int main()
{
    vector<double> x = {1000, 10000, 100000, 1000000};
    for(int i = 0; i < x.size(); i++)
    {   
        cout << "elements: " << fixed  << setprecision(0) << x[i] << endl;
        test_single_thread(x[i]);
        cout << endl;
        test_multi_thread(x[i]);
        cout << endl << endl;
    }
}
