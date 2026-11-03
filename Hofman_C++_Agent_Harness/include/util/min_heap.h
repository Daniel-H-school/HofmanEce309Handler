// include/util/min_heap.h
//
// YOU IMPLEMENT (this whole file; it is a template, so the code lives in the
// header). Spec §4.1.
//
// A binary MIN-heap stored in an array (a std::vector), plus top_k(), which
// uses a size-k min-heap to pick the k largest items in O(n log k).
//
// Rules for this file:
//   * Write sift-up and sift-down yourself.
//   * Do NOT use std::priority_queue, std::make_heap / push_heap / pop_heap /
//     sort_heap, std::sort / stable_sort / partial_sort / nth_element, or
//     std::set / multiset / map / multimap (full list: spec §7.3).
//   * std::vector is fine as the storage array; std::swap, std::reverse and
//     std::pair are fine too.
//
// `Less` is a "less-than" function object, exactly like the one std::sort
// takes: less(a, b) is true when a should come before b. The heap keeps the
// SMALLEST element (by less) at the top.

#pragma once
#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iostream> //Included for testing

template <typename T, typename Less = std::less<T>>
class MinHeap {
public:
    explicit MinHeap(Less less = Less()) : less_(std::move(less)) {}

    // Adds value. O(log n).
    void push(T value) {
        data_.push_back(value);    //appends to the end, expanding the array (vector) holding the heap behind the scenes
        std::size_t index = data_.size() - 1;   //Sets up index to be at appended value
        if (index == 0) return;     //Special case for the first element; would break [(index - 1) / 2] logic and access out of bounds
        //Sift up
        while (true){   //Internal logic ensures an exit
            if (index == 0){    //Once the value has been shifted all the way to the root, it must be the smallest.
                return;
            } else if (less_(value,data_[(index - 1) / 2])){  //The pushed value has a higher priority than its parent
                T temp = data_[(index - 1) / 2];    //Swaps into the higher value
                data_[(index - 1) / 2] = value;
                data_[index] = temp;
            }
            index = (index - 1) / 2;            //Index iterates into its parent. Uses integer division
        }
        (void)value;
    }

    // The smallest element. Throws std::out_of_range if empty. O(1).
    const T& top() const {
        if (data_.size()){  //If not an empty heap
            return data_[0];    //Returns top item without removing it
        } else {    //If an empty heap
            throw std::out_of_range("Attempted Access of Empty Minheap");
        }
    }

    // Removes and returns the smallest element. Throws std::out_of_range if
    // empty. O(log n).
    T pop() {
        // TODO: save the root, move the last element to the root, shrink,
        // sift down, return the saved root.
        // Clue: check empty() first. With one element, there is no "last
        // element" to move: just take it and pop_back().
        // Clue: T result = std::move(data_[0]); then move the last element
        // into slot 0 (moving avoids copying big strings).
        if (data_.size()){  //If not an empty heap
            T newRoot = data_[data_.size()-1];  //Assigns the newRoot to the last element
            T popped = data_[0];                //The poppped item to be removed
            data_[0] = newRoot;                 //Resets root
            data_.resize(data_.size() - 1);     //Removes the copy of the new root by resizing down
            
            //Sift down
            std::size_t index = 0;  //Index for sifting down
            while (true){   //While loop with defined break cases to avoid infinite looping
                if ((2 * index + 1) >= data_.size()){    //If the index past this one has no children
                    break;  //Since there are no children, this has sifted down fully
                } else if ((2 * index + 2) >= data_.size()){    //Special case for if there is only a left child (there cannot be only a right child)
                    if (less_(data_[2 * index + 1], newRoot)){  //If the newRoot is larger than the last child
                        data_[index] = data_[2 * index + 1];    //Swap the values
                        data_[2 * index + 1] = newRoot;
                        break;  //No more children after this one by definition
                    } else {
                        break;  //Sifted down fully
                    }
                }
                if (less_(data_[2 * index + 1], data_[2 * index + 2])){ //If the left child is less than the right
                    if (less_(data_[2 * index + 1],newRoot)){  //If the sifted value is more than this child
                        data_[index] = data_[2 * index + 1];    //Swap the values
                        data_[2 * index + 1] = newRoot;
                        index = 2 * index + 1;  //Move index down
                    } else {    //Else, the value has sifted down its max amount, as it is less than its lowest child
                        break;
                    }
                } else {    //If the right child is less than or equal to the left
                    if (less_(data_[2 * index + 2], newRoot)){  //If the sifted value is more than this child
                        data_[index] = data_[2 * index + 2];    //Swap the values
                        data_[2 * index + 2] = newRoot;
                        index = 2 * index + 2;  //Move index down
                    } else {    //Else, the value has sifted down its max amount
                        break;
                    }
                }
            }
            
            //After sifted down fully
            return popped;  //Return the popped value
        } else {    //If an empty heap
            throw std::out_of_range("Attempted Access of Empty MinHeap");
        }
    }

    std::size_t size() const noexcept { return data_.size(); }
    bool        empty() const noexcept { return data_.empty(); }
    void        clear() noexcept { data_.clear(); }

private:
    // ===== YOUR HELPERS AND MEMBERS =====
    // Children of index i are 2i+1 and 2i+2; its parent is (i-1)/2.
    // Suggested: void sift_up(std::size_t i);  void sift_down(std::size_t i);
    // Clue: use std::size_t (not int) for indices, or -Wsign-compare warns
    // every time you compare with data_.size(). Careful: (i-1)/2 with i == 0
    // underflows, so sift_up must stop at i == 0.
    // Clue: in sift_down, the right child may not exist: check
    // 2*i+2 < data_.size() before reading it. Swap with the SMALLER child.
    std::vector<T> data_;
    Less           less_;
};

// Returns the k LARGEST items of `items` (by `less`), largest first.
//   * k == 0 or items empty -> empty result
//   * k >= items.size()     -> every item, largest first
// Must keep a MinHeap of at most k items (the k largest seen so far, with
// the smallest of them on top), so it runs in O(n log k) time and O(k) extra
// space. Items that compare equal may come out in any order.
template <typename T, typename Less = std::less<T>>
std::vector<T> top_k(const std::vector<T>& items, std::size_t k, Less less = Less()) {
    // TODO: keep a MinHeap<T, Less> of at most k items. For each item: if
    // the heap has fewer than k items, push it; otherwise, if the heap's top
    // is less than the item, pop the top and push the item. At the end, pop
    // everything (smallest first) and return it reversed.
    //
    // Clues:
    //  * k == 0: return {} BEFORE the loop (top() on an empty heap throws).
    //  * Build the heap WITH the comparator:  MinHeap<T, Less> heap(less);
    //    Writing  MinHeap<T, Less> heap;  fails for lambdas ("use of deleted
    //    function ... lambda") because a lambda can't be default-constructed.
    //  * "top is less than the item" means  less(heap.top(), item),  not
    //    heap.top() < item.  Read less(a, b) as "a is WORSE than b": the heap
    //    keeps the k best items with the worst of them on top.
    //  * std::reverse (from <algorithm>) flips a vector.

    if (k == 0 || items.empty()){   //If given no items or told not to include any items
        return {};  //Returns empty vector early
    }

    MinHeap<T, Less> processHeap;        //Initializes a minHeap with the current type and comparison method
    for (T item : items){       //For each item in items
        if (processHeap.size() < k){   //If there is room in the minHeap for more items
            processHeap.push(item); //Pushes item to the minHeap
        } else {    //If there is not enough room in the minHeap,
            if (less(processHeap.top(), item)){    //If the item is larger than the smallest value in the process heap
                processHeap.pop();      //Pop the smallest element
                processHeap.push(item); //Push the element replacing it
            }   //Otherwise, moves on without doing anything
        }
    }

    //After the k-most items have been processed
    std::vector<T> outputVector;    //Initializes a vector to hold the output
    for (std::size_t i = k; i > 0; i--){    //For k items (processheap may hold more than k items)
        outputVector[i - 1] = processHeap.pop();     //Pop the smallest item into the output vector, starting at the smallest and building towards the largest
        //The largest vlaue is at index zero
    }

    //Because of the structure of minHeaps, the two children of a node may not be in order, so has to go through and swap conflicting ones
    for (std::size_t i = 1; i < k; i+=2){   //Starts at index 1 and iterates by two to compare two children at a time
        if (i + 1 > k) break;   //Fringe out of array check. Will be at the end of the vector and means there is an unpaired child which doesn't need swapping anyway
        else if (less(outputVector[i],outputVector[i + 1])){    //If left child is larger than right child
            T temp = outputVector[i];               //Swap
            outputVector[i] = outputVector[i + 1];
            outputVector[i + 1] = temp;
        }
    }

    return outputVector;    //Return the output vector

    (void)items;
    (void)k;
    (void)less;
}
