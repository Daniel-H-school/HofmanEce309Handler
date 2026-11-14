// src/memory/memory_index.cpp — YOU IMPLEMENT. Spec §4.3.

#include "memory/memory_index.h"

MemoryIndex::~MemoryIndex() {
    // TODO: free every node (post-order: both children before the node).
    (void)root_;
}

//Implementing this as an AVL for bonus (maybe)
bool MemoryIndex::insert(MemoryRecord rec) {
    // TODO: walk down from root_ (smaller ids left, larger right). Equal id:
    // return false and change nothing. Otherwise attach a new Node where the
    // walk fell off the tree, ++count_, return true.
    // Clue: memory ids arrive as 1, 2, 3, ..., so the tree is one long right
    // spine. A loop handles 5,000 nodes easily; so does recursion.
    Node* curNode = root_;

    if (root_ == nullptr){  //For the first node
        Node* newNode = new Node{rec,nullptr,nullptr};  //Put in a new node into the BST
        root_ = newNode;
        count_++;       //Increment count
        return true;
    }

    while (true){ //handles logic for traversing and inserting
        if (curNode->rec.id == rec.id){
            return false;       //If this id already inside, do nothing
        } else if (curNode->rec.id > rec.id){   //Traverses left
            if (curNode->right == nullptr) {
                curNode->right = new Node{rec,nullptr,nullptr}; //creates a node to put in this place
                count_++;       //Increments count
                return true;    //Has inserted
            }
            curNode = curNode->right;
        } else {
            if (curNode->left == nullptr) {
                curNode->left = new Node{rec,nullptr,nullptr};  //creates a node to put in this place
                count_++;       //Increments count
                break;    //Breaks out when it finds where to insert
            }
            curNode = curNode->left;
        }
    }
    
}

const MemoryRecord* MemoryIndex::find(std::uint64_t id) const {
    // TODO
    Node* curNode = root_;
    if (curNode = nullptr) return nullptr;  //Return nullptr of trying to find an empty bst
    while (curNode != nullptr){     //Traversal
        if (curNode->rec.id == id){
            return &curNode->rec;   //Return pointer to the memory record after it has been found
        } else if (curNode->rec.id > id) {
            curNode = curNode->right;
        } else {
            curNode = curNode->left;
        }
    }
    return nullptr; //Node not found
}

bool MemoryIndex::remove(std::uint64_t id) {
    // TODO: leaf, one child, two children (use the in-order successor or
    // predecessor). Spec Appendix C draws all three.
    // Clue: the successor (leftmost node of the right subtree) has no left
    // child but MAY have a right child: that child takes its place.
    // Clue: --count_ exactly once per successful remove. Counting inside
    // the helper AND here double-counts the two-children case.
    // Clue: when the removed node is the root, root_ itself must change.
    
    Node* curNode = root_;
    Node* prevNode = root_;
    if (curNode = nullptr) return false; //No id found because bst is empty
    while (curNode != nullptr){ //Traversal
        if (curNode->rec.id == id){ //Found this id
            if (curNode->left == nullptr && curNode->right == nullptr){ //if a leaf
                delete curNode; //Deleted for memory management
            } else if (curNode->left == nullptr){   //If this has only a right child
                if (prevNode->left == curNode){ //If this was a left child
                    prevNode->left = curNode->right;   //Skipp over the cur node
                    delete curNode;
                    count_--;
                    return true;
                } else {    //if this was a right child
                    prevNode->right = curNode->right;
                    delete curNode;
                    count_--;
                    return true;
                }  
            } else if (curNode->right == nullptr){  //If this has only a left child
                if (prevNode->left == curNode){ //If this was a left child
                    prevNode->left = curNode->left;
                    delete curNode;
                    count_--;
                    return true;
                } else {
                    prevNode->right = curNode->left;
                    delete curNode;
                    count_--;
                    return true;
                }
            } else {    //Else if there are two children
                Node* successor = curNode->right;
                Node* prevSucc = successor;
                while (successor->left != nullptr){ //Traverses to find the smallest value on the right subtree
                    prevSucc = successor;
                    successor = successor->left;
                }
                prevSucc->left = successor->right;  //Replaces successor down the tree
                if (prevNode->left == curNode){ //If this was a left child
                    prevNode->left = successor;         //Relink up successor
                    successor->left = curNode->left;
                    successor->right = curNode->right;
                    delete curNode;                     //free up deleted node
                    count_--;
                    return true;
                }  else {   //If this was a right child
                    prevNode->right = successor;
                    successor->left = curNode->left;
                    successor->right = curNode->right;
                    delete curNode;
                    count_--;
                    return true;
                }
            }
        } else if (curNode->rec.id > id){
            prevNode = curNode;
            curNode = curNode->right;
        } else {
            prevNode = curNode;
            curNode = curNode->left;
        }
    }
    //otherwise, it wasn't found
    return false;
}

std::size_t MemoryIndex::size() const noexcept { return count_; }

std::size_t MemoryIndex::height() const {
    // TODO: empty tree = 0, one node = 1, otherwise 1 + the taller child.
    if (root_ == nullptr) return 0;
    Node* curNode = root_;
    //Pick up here!!!!
    //Helper function for height of a node recursive implementation?

    return 0;
}

void MemoryIndex::for_each_in_order(const std::function<void(const MemoryRecord&)>& fn) const {
    // TODO: left subtree, then fn(node->rec), then right subtree.
    (void)fn;
}

void MemoryIndex::clear() {
    // TODO
    // Clue: after freeing the nodes, set root_ = nullptr and count_ = 0, or
    // the destructor frees them a second time.
}
