// src/memory/memory_index.cpp — YOU IMPLEMENT. Spec §4.3.

#include "memory/memory_index.h"

MemoryIndex::~MemoryIndex() {
    // TODO: free every node (post-order: both children before the node).
    clear();
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
        } else if (curNode->rec.id < rec.id){   //Traverses right
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
                return true;    //Breaks out when it finds where to insert
            }
            curNode = curNode->left;
        }
    }
    
}

const MemoryRecord* MemoryIndex::find(std::uint64_t id) const {
    // TODO
    Node* curNode = root_;
    if (curNode == nullptr) return nullptr;  //Return nullptr of trying to find an empty bst
    while (curNode != nullptr){     //Traversal
        if (curNode->rec.id == id){
            return &curNode->rec;   //Return pointer to the memory record after it has been found
        } else if (curNode->rec.id < id) {
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
    Node* prevNode = nullptr;
    
    //First, finds the correct node
    while (curNode != nullptr){
        if (curNode->rec.id == id){         //If it finds the node
            break;
        } else if (curNode->rec.id < id){   //Else, move left or right
            prevNode = curNode;         
            curNode = curNode->right;
        } else {
            prevNode = curNode;
            curNode = curNode->left;
        }
    }
    
    if (curNode == nullptr) return false;   //If the desired node isn't found, return false early
    if (prevNode == nullptr){    //Special case for deleting root_
        Node* succ = curNode->right;
        Node* prevSucc = nullptr;
        if (succ == nullptr){   //If no successor (only has a left child, or none)
            if (curNode->left == nullptr){  //If root is only node
                delete curNode;
                root_ = nullptr;
                count_--;
                return true;     //Return early
            } else {
                root_ = curNode->left;  //Resets root_
                count_--;
                delete curNode;
                return true;     //Return early
            }
        }
        while (succ->left != nullptr){  //traverses down
            prevSucc = succ;
            succ = succ->left;
        }
        if (prevSucc == nullptr){   //Special case for successor simply being right pointer
            root_ = succ;       //Difference: sets root_
            succ->left = curNode->left; //replaces curNode's position. In this case, right pointer does not need to change
            delete curNode;
            count_--;
        } else {
            prevSucc->left = succ->right;   //Successor always left child, replaces link
            root_ = succ;   //Sets root_
            succ->left = curNode->left;     //Replaces curNode with successor
            succ->right = curNode->right;
            delete curNode;
            count_--;
        }
    } else if (curNode->left == nullptr && curNode->right == nullptr){ //If the node to be deleted is a leaf node
        if (prevNode->left == curNode){ //If the removed node was a left child
            prevNode->left = nullptr;   //Null out connection
            delete curNode;             //Delete removed node for space management
            count_--;
        } else {                        //Removed node was a right child
            prevNode->right = nullptr;
            delete curNode;
            count_--;
        }
    } else if (curNode->left == nullptr){   //If the removed node only has a right child
        if (prevNode->left == curNode){
            prevNode->left = curNode->right;
            delete curNode;
            count_--;
        } else {
            prevNode->right = curNode->right;
            delete curNode;
            count_--;
        }
    } else if (curNode->right == nullptr){  //If the removed node only has a left child
        if (prevNode->left == curNode){
            prevNode->left = curNode->left;
            delete curNode;
            count_--;
        } else {
            prevNode->right = curNode->left;
            delete curNode;
            count_--;
        }
    } else {    //Two children case with successor system
        Node* succ = curNode->right;
        Node* prevSucc = nullptr;
        while (succ->left != nullptr){    //Finds left-most element of right subtree
            prevSucc = succ;
            succ = succ->left;
        }
        if (prevSucc == nullptr){   //Special case for the successor simply being the first node found
            if (curNode == prevNode->left){ //Left child
                prevNode->left = succ;
                succ->left = curNode->left;
                //Succ->right should stay the same value (nullptr or whatever)
                delete curNode;
                count_--;
            } else {                        //right child
                prevNode->right = succ;
                succ->left = curNode->left;
                delete curNode;
                count_--;
            }
        } else {
            prevSucc->left = succ->right;   //Successor is always a left child and is here replaced
            succ->left = curNode->left;
            succ->right = curNode->right;
            if (prevNode->left == curNode){ //If curNode was a left child
                prevNode->left = succ;      //Successor replace
            } else {
                prevNode->right = succ;
            }
            delete curNode;
            count_--;
        }
    }
    return true;

}

std::size_t MemoryIndex::size() const noexcept { return count_; }

std::size_t MemoryIndex::height() const {
    // TODO: empty tree = 0, one node = 1, otherwise 1 + the taller child.
    if (root_ == nullptr) return 0;
    return height_of(root_);    //Currently uses a recursive approach, be wary at high data structure sizes

}

void MemoryIndex::for_each_in_order(const std::function<void(const MemoryRecord&)>& fn) const {
    // TODO: left subtree, then fn(node->rec), then right subtree.
    in_order(root_,fn); //uses iterative helper function to apply function to each nodes' rec
}

void MemoryIndex::clear() {
    // TODO
    // Clue: after freeing the nodes, set root_ = nullptr and count_ = 0, or
    // the destructor frees them a second time.
    if (root_ == nullptr) return;   //Doesn't call the delete if already deleted
    post_order_destroy(root_);  //Calls destroy on the root
    root_ = nullptr;            //changes root ptr to avoid pointing to deleted memory
    count_ = 0;                 //Resets count
}
