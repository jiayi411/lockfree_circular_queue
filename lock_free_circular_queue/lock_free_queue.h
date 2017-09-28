//
//  lock_free_queue.h
//  lock_free_circular_queue
//
//  Created by jiayi on 27/09/2017.
//  Copyright © 2017 jiayi. All rights reserved.
//

#ifndef lock_free_queue_h
#define lock_free_queue_h

#include <atomic>
#include <vector>
#include <assert.h>

template<typename _T, std::size_t _Size>
class lock_free_queue
{
public:
    typedef _T value_type;

public:
    lock_free_queue() : head__(0), tail__(0){}
    ~lock_free_queue() = default;
    lock_free_queue(const lock_free_queue&) = delete;
    lock_free_queue& operator= (const lock_free_queue&) = delete;
    lock_free_queue(lock_free_queue&& in_queue) noexcept {
        *this = in_queue;
    }
    lock_free_queue& operator= (lock_free_queue&& in_queue) noexcept {
        head__ = in_queue.head__;
        tail__ = in_queue.tail__;
        datas__ = in_queue.datas__;
        return *this;
    }
    
public:
    bool try_push(const value_type& item) {
        if (isFull()) { return false; }
        
        const int tail = tail__.load(std::memory_order_relaxed);
        datas__[tail] = item;
        tail__.store(increment(tail), std::memory_order_release);
        return true;
    }
    
    void push(const value_type& item) {
        while (!try_push(item)) {}
    }
    
    bool try_pop() {
        if (isEmpty()) { return false; }
        
        const int head = head__.load(std::memory_order_relaxed);
        head__.store(increment(head), std::memory_order_release);
        return true;
    }
    void pop() {
        while (!try_pop()) {}
    }
    
    bool isEmpty() const {
        // when head == tail, it is empty
        return head__.load(std::memory_order_acquire) == tail__.load(std::memory_order_acquire);
    }
    
    bool isFull() const {
        // when tail + 1 == head, it is full
        const int tail = tail__.load(std::memory_order_acquire);
        const int head = head__.load(std::memory_order_acquire);
        if (increment(tail) ==  head) {
            return true;
        }
        return false;
    }
    
    int count() const {
        const int tail = tail__.load(std::memory_order_acquire);
        const int head = head__.load(std::memory_order_acquire);
        if (tail == head) { assert(tail == head); return 0; }
        if (tail > head) { assert(tail > head); return tail - head; }
        assert(tail < head);
        return _Size - head + tail;
    }
    
    const value_type& front() const {
        return datas__[head__.load(std::memory_order_acquire)];
    }
    
    value_type& front() {
        return datas__[head__.load(std::memory_order_acquire)];
    }
    
    const value_type& back() const {
        return datas__[tail__.load(std::memory_order_acquire)];
        
    }
    value_type& back() {
        return datas__[tail__.load(std::memory_order_acquire)];
    }
    
private:
    int increment(const int n) const {
        return (n + 1) % _Size;
    }
    
private:
    std::atomic<int> head__;    ///< head of queue
    std::atomic<int> tail__;    ///< tail of queue
    value_type datas__[_Size];          ///< datas
};

#endif /* lock_free_queue_h */
