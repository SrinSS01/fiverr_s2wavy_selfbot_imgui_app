#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <iostream>
#include "types/Tag.h"

template<typename T> class LinkedList {
private:
    template<typename K>
    struct Node {
        K data;
        std::shared_ptr<Node<K>> next;
        std::shared_ptr<Node<K>> previous;
    };
    mutable std::mutex mtx;

public:
    std::shared_ptr<Node<T>> head;
    std::shared_ptr<Node<T>> last;
    bool is_valid;

    LinkedList();
    ~LinkedList();
    void push_front(T value);
    void push_back(T value);
    void pop_back();
    void for_each(std::function<void(T&)> const& f);
    void clear();
    [[nodiscard]] bool is_empty() const;
    LinkedList<T>& operator=(LinkedList<T> const& list);
    void delete_value(T &t);
};

template<typename T>
void LinkedList<T>::delete_value(T &t) {
    if (!head) {
        return;
    }

    // Start from the head
    std::shared_ptr<Node<T>> current = head;
    std::shared_ptr<Node<T>> prev = nullptr;

    while (current) {
        if (current->data == t) {
            // Found the node with the specified t, remove it
            if (prev) {
                // Update the previous node's next pointer
                prev->next = current->next;
            } else {
                // If the head node matches, update the head pointer
                head = current->next;
            }

            // If the last node matches, update the last pointer
            if (current == last) {
                last = prev;
            }

            // Delete the node
            current.reset();
            return; // Exit the function
        }

        // Move to the next node
        prev = current;
        current = current->next;
    }
}

template<typename T>
LinkedList<T>& LinkedList<T>::operator=(const LinkedList<T> &list) {
    this->head = list.head;
    this->last = list.last;
    this->is_valid = list.is_valid;
    return *this;
}

template<typename T>
LinkedList<T>::LinkedList() : head(nullptr), last(nullptr), is_valid(true) {}

template<typename T>
LinkedList<T>::~LinkedList() {
    std::lock_guard<std::mutex> lock(mtx);
    while (head != nullptr) {
        auto temp = head;
        head = head->next;
        temp->previous = nullptr; // Disconnect the node from the list
        temp->next = nullptr; // Ensures that the shared_ptr<Node> will be destructed
    }
    last = nullptr; // Ensure the last pointer is also cleared
}

template<typename T>
void LinkedList<T>::push_front(T value) {
    std::lock_guard<std::mutex> lock(mtx);
    auto new_node = std::make_shared<Node<T>>();
    new_node->data = value;
    new_node->next = head;
    if (head != nullptr) {
        head->previous = new_node;
    }
    head = new_node;
    if (last == nullptr) { // List was empty
        last = new_node;
    }
}

template<typename T>
void LinkedList<T>::push_back(T value) {
    std::lock_guard<std::mutex> lock(mtx);
    auto new_node = std::make_shared<Node<T>>();
    new_node->data = value;
    new_node->previous = last;
    if (last != nullptr) {
        last->next = new_node;
    }
    last = new_node;
    if (head == nullptr) { // List was empty
        head = new_node;
    }
}

template<typename T>
void LinkedList<T>::pop_back() {
    std::lock_guard<std::mutex> lock(mtx);
    if (last == nullptr) { // List is empty
        return;
    }
    last = last->previous;
    if (last != nullptr) {
        last->next = nullptr;
    } else {
        head = nullptr; // List is now empty
    }
}

template<typename T>
void LinkedList<T>::for_each(std::function<void(T&)> const& f) {
//    std::lock_guard<std::mutex> lock(mtx);
    for (auto node = head; node != nullptr; node = node->next) {
        if (!is_valid) {
//            std::cout << "Stopping because invalid." << std::endl;
            clear();
            return;
        }
        f(node->data);
    }
}

template<typename T>
void LinkedList<T>::clear() {
    if (is_empty()) { return; }
    std::lock_guard<std::mutex> lock(mtx);
    while (head != nullptr) {
        auto temp = head;
        head = head->next;
        temp->previous.reset(); // Disconnect the node from the list
        temp->next.reset(); // Ensures that the shared_ptr<Node> will be destructed
    }
    last.reset(); // Ensure the last pointer is also cleared
}

template<typename T>
bool LinkedList<T>::is_empty() const {
    return head == nullptr;
}