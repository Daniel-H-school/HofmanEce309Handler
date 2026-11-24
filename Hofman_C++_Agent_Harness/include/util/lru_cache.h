// include/util/lru_cache.h
//
// YOU IMPLEMENT (this whole file; it is a template). Spec §4.2.
//
// A fixed-capacity cache that evicts the Least Recently Used entry.
// Both get() and put() must be O(1) (expected), so it is built from two
// structures that point at the same nodes:
//
//   std::unordered_map<Key, Node*>   finds a key's node in O(1)
//   a doubly linked list of Nodes    keeps recency order:
//        head_ = most recently used ... tail_ = least recently used
//
// Rules for this file:
//   * Write the doubly linked list yourself (Node with prev/next pointers,
//     new/delete inside this class only). Do NOT use std::list, std::deque
//     or std::forward_list (don't even #include their headers), or a
//     std::vector you shift around (spec §7.3).
//   * std::unordered_map is allowed (and expected) for the key -> Node* index.
//   * No leaks, no double frees: ASan runs on every test.

#pragma once
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream> //For debugging

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class LruCache {
public:
    struct Stats {
        std::size_t hits = 0;        // get() found the key
        std::size_t misses = 0;      // get() did not find the key
        std::size_t evictions = 0;   // put() removed the LRU entry to make room
    };

    // A cache holding at most `capacity` entries. Capacity 0 is legal: such a
    // cache never stores anything (every get() is a miss).
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {}

    // Frees every node.
    ~LruCache() { clear(); }

    // Copying a cache of raw-pointer nodes is not needed; forbid it.
    LruCache(const LruCache&) = delete;
    LruCache& operator=(const LruCache&) = delete;

    // Hit: marks the entry most recently used, counts a hit, and returns a
    //      pointer to its value (valid until the next put/erase/clear).
    // Miss: counts a miss and returns nullptr.
    const Value* get(const Key& key) {
        // TODO
        // Clue: look keys up with  auto it = map_.find(key);  and compare
        // with map_.end(). Do NOT use map_[key] to look up: on a miss it
        // INSERTS the key (with a nullptr node), size() grows, and a later
        // put() crashes far away from the real bug.  
        auto it = map_.find(key);
        if (it == map_.end()){   //Checks if the desired key is even in the hash
            stats_.misses++;        //Increments misses
            return nullptr;         //Returns nullptr early
        }
        move_to_front(map_.at(key));    //moves this address to front (helper function)
        stats_.hits++;      //Increments hits stat
        return &(map_.at(key)->value);  //Returns a pointer to the value of the node at that location
    }

    // Key present: replaces its value and marks it most recently used (no
    //   eviction, no hit/miss counted).
    // Key absent: if the cache is full, evicts the least recently used entry
    //   (counting one eviction), then inserts the new entry as most recently
    //   used. With capacity 0, does nothing.
    void put(const Key& key, Value value) {
        // TODO
        // Clue (eviction): victim = tail_; unlink it; erase victim->key from
        // the map; THEN delete victim. Reading victim->key after delete is a
        // heap-use-after-free (ASan may show it as a SEGV inside hashtable
        // code: look for the first stack frame in lru_cache.h).
        // Clue: create nodes with  new Node{key, std::move(value), nullptr, nullptr}.
        Node* newNode = new Node{key, std::move(value), nullptr, nullptr};  //Creates a new node with this key and value
        if (map_.find(key) == nullptr){   //If the key is absent
            if (capacity_ == 0){    //If capacity is zero, do nothing
                delete newNode; //Prevent memory leak
            } else if (size() == capacity_){   //If the cache is full
                push_front(newNode);    //Pushes the new node to front
                Node* oldTail = unlink(tail_);  //Unlink the tail pointer
                map_.erase(oldTail->key);    //Erase the old node in hash
                map_[key] = newNode;   //Place the new node in hash

                delete oldTail; //Frees the memory of this now unused Node
                stats_.evictions++; //Increments evictions
            } else {
                push_front(newNode);    //Pushes the new node to front
                map_[key] = newNode;   //place the new node in map
            }
        } else {    //If the key is present
            Node* valAddr = map_.at(key);  //Get the address of the value at the key
            valAddr->value = value;  //Set the Value of that key to be the new value
            move_to_front(valAddr); //Moves the node to the front (most recently used)
            delete newNode;         //Delete the memory associated with the node which wasn't created
        }
    }

    // True if the key is cached. Does NOT change recency or stats.
    bool contains(const Key& key) const {
        // TODO
        // Clue: this function is const, so map_[key] does not even compile
        // ("discards qualifiers"). Use map_.find(key) != map_.end() or
        // map_.count(key).
        if (map_.find(key) == map_.end()){  //if the key is not cached
            return false;
        } else {    //The key is cached
            return true;
        }
    }

    // Removes the key if present (not an eviction). Returns whether it was.
    bool erase(const Key& key) {
        if (map_.empty() || (map_.find(key) == map_.end())){  //if the key is not cached OR if map has no items in it to delete
            return false;
        } else {    //if the key is cached and needs to be erased
            Node* toDelete = map_.at(key);
            unlink(toDelete);   //Unlink this node
            map_.erase(toDelete->key);  //Erase from hash
            delete toDelete;    //Free this node's data
            return true;
        }
        
    }

    // Removes every entry. Stats are kept.
    void clear() {
        // TODO: delete every node, empty the map, reset head_/tail_.
        // Clue: save n->next in a local BEFORE  delete n;  a loop like
        //   for (Node* n = head_; n; n = n->next) delete n;
        // reads n->next from freed memory.

        if (head_ == nullptr){   //If there is an empty list
            //Do nothing
        } else {
            Node* curNode = head_->next;    //Sets the current node to be one ahead of the node deletion
            while (curNode != nullptr){     //While there is still a current node
                delete (curNode->prev);     //Delete the previous node as it goes
                curNode = curNode->next;    
            }
            //When the curNode was null, the previous node was the Tail
            delete tail_;
            head_ = nullptr;    //Head and tail pointer reset
            tail_ = nullptr;    //head and tail pointer reset
        }
    }

    std::size_t size() const noexcept { return map_.size(); }
    std::size_t capacity() const noexcept { return capacity_; }
    Stats       stats() const noexcept { return stats_; }

    // Keys from most to least recently used, by walking the list from head_.
    // Used by the tests to check your recency order.
    std::vector<Key> keys_mru_to_lru() const {
        std::vector<Key> out;
        for (Node* n = head_; n != nullptr; n = n->next) out.push_back(n->key);
        return out;
    }

private:
    struct Node {
        Key   key;
        Value value;
        Node* prev;
        Node* next;
    };

    // ===== YOUR HELPERS AND MEMBERS =====
    // Suggested helpers: unlink(Node*), push_front(Node*), move_to_front(Node*).
    // Clue for unlink(n): four cases. If n has a prev, prev->next skips n,
    // otherwise n was head_ and head_ moves to n->next. Likewise, if n has
    // a next, next->prev skips n, otherwise n was tail_ and tail_ moves to
    // n->prev. Unlinking the only node leaves head_ == tail_ == nullptr.
    // Spec Appendix B lists every pointer write.

    //Helper function to push something new to the most recently used edge
    void push_front(Node* n){
        //Places this node at front, assuming that it isn't connected to anything
        if (head_ == nullptr){  //In case there isn't a head (nor tail)
            head_ = n;
            tail_ = n;
        } else { //All other cases
            n->next = head_;
            head_->prev = n;
            head_ = n;
        }
    }

    //Helper function to move a node to the most recently used
    void move_to_front(Node * n){
        unlink(n);
        push_front(n);
    }

    //Helper function to unlink a node (used in move_to_front) and eviction
    Node* unlink (Node* n){
        if (head_ == n && tail_ == n){  //If this is specificalaly the only node
            head_ = nullptr;
            tail_ = nullptr;
        }
        if (n->next == nullptr && n->prev == nullptr){  //If this is already unlinked, do nothing

        } else if (n->prev == nullptr){ //If this was the head node
            head_ = n->next;            //New head ptr
            n->next = nullptr;          //Free the links
            head_->prev = nullptr;     
        } else if (n->next == nullptr){
            tail_ = n->prev;            //New tail ptr
            tail_->next = nullptr;      //Free the links
            n->prev = nullptr;
        } else {    //Node in middle of list
            n->prev->next = n->next;    //Skip this node going forward
            n->next->prev = n->prev;    //Skip this node going back
            n->next = nullptr;
            n->prev = nullptr;
        }
        return n;   //Returns n in order for it to get deleted
    }

    std::unordered_map<Key, Node*, Hash> map_;
    Node*       head_ = nullptr;
    Node*       tail_ = nullptr;
    std::size_t capacity_;
    Stats       stats_;
};
