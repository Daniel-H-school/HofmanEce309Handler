// include/memory/memory_index.h
//
// YOU IMPLEMENT (src/memory/memory_index.cpp). Spec §4.3.
//
// A binary search tree of memory records, keyed by id. The provided
// MemoryStore uses it for remember (insert), forget (remove), and
// list_memories (in-order traversal).
//
// Rules: write the BST yourself with Node pointers. Do NOT use std::map,
// std::set, std::unordered_map, or any other associative container, and do
// not keep the records in a sorted std::vector. The tree does not need to be
// balanced (balancing it as an AVL tree is the bonus, spec §9).
//
// The PUBLIC interface is fixed. The PRIVATE section is yours.

#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <iostream>

struct MemoryRecord {
    std::uint64_t id = 0;     // unique key; shown to the model as "m<id>"
    std::string   text;
};

class MemoryIndex {
public:
    MemoryIndex() = default;
    ~MemoryIndex();                       // frees every node

    MemoryIndex(const MemoryIndex&) = delete;
    MemoryIndex& operator=(const MemoryIndex&) = delete;

    // Adds `rec`. Returns false (and changes nothing) if a record with the
    // same id is already stored.
    bool insert(MemoryRecord rec);

    // The record with this id, or nullptr. The pointer is valid until the
    // next insert/remove/clear.
    const MemoryRecord* find(std::uint64_t id) const;

    // Removes the record with this id. Returns false if there is none.
    // Handles all three cases: leaf, one child, two children.
    bool remove(std::uint64_t id);

    std::size_t size() const noexcept;

    // Number of nodes on the longest root-to-leaf path: 0 for an empty
    // tree, 1 for a single node.
    std::size_t height() const;

    // Calls fn on every record in ascending id order.
    void for_each_in_order(const std::function<void(const MemoryRecord&)>& fn) const;

    // Removes every record.
    void clear();

private:
    // ===== YOUR REPRESENTATION GOES HERE =====
    // Suggested: a Node struct holding a MemoryRecord and left/right
    // pointers, a root pointer, and a node count (below), plus helpers that
    // you declare here and define in memory_index.cpp, for example:
    //
    //   static void        destroy(Node* n);              // post-order delete
    //   static std::size_t height_of(const Node* n);
    //   static void        in_order(const Node* n,
    //                               const std::function<void(const MemoryRecord&)>& fn);
    //   static Node*       remove_node(Node* n, std::uint64_t id, bool* removed);
    //       // returns the new root of this subtree, so the caller writes
    //       //   root_ = remove_node(root_, id, &removed);
    //
    // Clue: a static helper cannot touch count_. Change count_ in the public
    // insert()/remove() instead, exactly once per success.
    // Clue: in the .cpp, a helper that RETURNS Node* must be written
    //   MemoryIndex::Node* MemoryIndex::remove_node(...)
    // (plain "Node*" there gives "'Node' does not name a type").
    struct Node {
        MemoryRecord rec;
        Node*        left = nullptr;
        Node*        right = nullptr;
    };

    //recursive helper for the height function
    static std::size_t height_of(Node* n){
        if (n == nullptr){
            return 0;
        }
        struct stack {
            Node* node;
            std::size_t nodeHeight;
        };
        std::vector<stack> fakeStack;   //Vector that imitates a runtime stack for iterative recursion
        Node* curNode = n;
        std::size_t curNodeHeight = 1;
        std::size_t height = 1;
        std::size_t maxHeight = 1;
        while (!fakeStack.empty() || curNode != nullptr){ //While there are nodes to process
            while (curNode != nullptr){     //travel all the waay left on this subchild
                fakeStack.push_back({curNode,height});
                //Different counting logic
                if (curNode->left != nullptr) {
                    height = curNodeHeight + 1;       //Increments height for traveling down this subpath
                    curNodeHeight++;
                } 
                curNode = curNode->left;    
                if (height > maxHeight) maxHeight = height; //Updates when a higher height value is found
            }
            curNode = fakeStack.back().node;     //Similar logic to in_order function. Pops off the stack to investigate child/right subpath
            height = fakeStack.back().nodeHeight;   //Gets height when traversing back
            fakeStack.pop_back();

            if (curNode->left == nullptr && curNode->right == nullptr) {
            }
            curNode = curNode->right;
            if (curNode != nullptr) {
                height++;   //increases height again when moving right successfully
                curNodeHeight = height;
            } 
            if (height > maxHeight) maxHeight = height; //Updates when a higher height value is found
        }
        return maxHeight;   //Max height consistently overestimates by one because of starting at root instead of leaf, so this corrects for it

    }

    //iterative helper for the in_order function (recursive function caused stack overfloww)
    static void in_order(const Node* n, const std::function<void(const MemoryRecord&)>& fn){
        if (n == nullptr) return;

        std::vector<const Node*> fakeStack;   //Order vector that places nodes in order iteratively 
        const Node* curNode = n;
    
        while (!fakeStack.empty() || curNode != nullptr){     //Stack based iteration (the or covers the root case)
            while (curNode != nullptr){ //Travel all the way left until you can't
                fakeStack.push_back(curNode);   //Push to stack
                curNode = curNode->left;        //Travel left
            }
            //Now, curNode is as far left as it can be (nullptr), so get the node from the stack
            curNode = fakeStack.back();
            fakeStack.pop_back();
            fn(curNode->rec);   //Perform the function on this node

            curNode = curNode->right;   //Go to right subtree and repeat
                                        //when this eventually reaches a nullptr itself (leaf), it will pop back up one layer to the parent
                                        //When the parent is the root, fakeStack will be empty and the while loop closes
        }

    }

    //Iterative helper for the clear function. Post_order traaversal to delete every node
    //RENAME BEFORE FINAL; IS NO LONGER POST_ORDER
    void post_order_destroy(Node* n){
        if (n == nullptr) return;

        std::vector<Node*> fakeStack;
        fakeStack.push_back(n);

        //Uses pseudo breadth-first approach to stack all children of a node onto the stack before deletion
        while (!fakeStack.empty()){
            //Pops the top of stack. things are not restacked if deleted, so this should process everything
            Node* curNode = fakeStack.back();
            fakeStack.pop_back();

            //Pushes children of currentNode, if any
            if (curNode->left != nullptr){
                fakeStack.push_back(curNode->left);
            }
            if (curNode->right != nullptr){
                fakeStack.push_back(curNode->right);
            }

            delete curNode;
        }
    }

    Node*       root_ = nullptr;
    std::size_t count_ = 0;
};
