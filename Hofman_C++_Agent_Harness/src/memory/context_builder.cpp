// src/memory/context_builder.cpp — YOU IMPLEMENT. Spec §4.4.

#include "memory/context_builder.h"
#include "util/min_heap.h"
#include <iostream>

ContextPlan select_context(const std::vector<ContextItem>& items,
                           std::size_t budget, std::size_t keep_recent) {
    // TODO: total the tokens; if they fit, keep everything. Otherwise push
    // every unprotected item onto a MinHeap keyed on (drop_rank(role), index)
    // and pop/drop until the total fits or the heap is empty.
    //
    // Clues:
    //  * Heap entries:  MinHeap<std::pair<int, std::size_t>> heap;
    //    std::pair compares .first, then .second, which is exactly
    //    "lowest rank first, then oldest first".
    //  * "The last keep_recent items" start at index items.size() - keep_recent,
    //    but these are unsigned: if keep_recent > items.size() that
    //    subtraction wraps around to a huge number. Handle that case first.
    //  * A vector<bool> keep(items.size(), true) makes the final pass easy:
    //    collect the indices still marked true, in ascending order.
    //  * total_tokens is the sum of the KEPT items; over_budget is
    //    total_tokens > budget.
    
    MinHeap<std::pair<int, std::size_t>> heap;  //minHeap used to store and compare the items
    std::vector<std::size_t> keep_indeces;
    ContextPlan output = {keep_indeces};

    //For every ContextItem
    std::size_t index = 0;
    for (auto item : items){
        keep_indeces.push_back(index);  //Push through all items
        output.total_tokens += item.tokens; //Update the total tokens count
        if (item.pinned) {heap.push({drop_rank(Role::System), index});}   //Internally treats pinned messages like system messages for logic purposes so they are never dropped
        else {heap.push({drop_rank(item.role),index});}    //Update minHeap with actual role if not pinned
        index++;    //Iterate index
    }
    index = 0;  //Resets index for later use
    if (output.total_tokens <= budget){    //If all the items can fit within the budget anyways
    } else if (keep_recent >= items.size()){   //Escape for if all items are within keep_recent command and should all be kept, but over the budget
        output.over_budget = true;      //By definition of this case, will be over budget
    } 
    //Else, have to start implementing logic
    //Don't want to keep all items, and is over budget
    else {
        std::vector<bool> keep(keep_indeces.size(),true);   //Initializes a vector to mark which indeces get dropped. True = keep
        while (output.total_tokens > budget){   //While we are still over budget
            auto nextRemove = heap.pop();   //The top of the heap for use in comparison
            if (nextRemove.first == 3){ //If a system message which cannot be dropped or something within the most recent convo
                output.over_budget = true;  //Mark that we are over budget
                break;  //Continue with next index, because of recency keeps and pinned messages
            } else if (nextRemove.second >= (items.size() - keep_recent)) {  //Else if this item happens to be within the most recent, continue while loop, but keep going until you hit the pinned or system messages which must be kept
                continue;  //Continue with next index, because of recency keeps and pinned messages                
            } else {    //Else, remove this index
                keep[nextRemove.second] = false; //Set this index not to go forward
                output.total_tokens -= items[nextRemove.second].tokens;  //Decrement token count from this item
            }
        }
        std::size_t j = 0;  //Index for insertion; i is index for comparison
        for (std::size_t i = 0; i < keep_indeces.size(); i++){  //For all items in the keep reference vector
            if (keep[i]){
                keep_indeces[j] = keep_indeces[i];  //Since j always is <= i, this will lag behind
                j++;
            }
        }
        keep_indeces.resize(j);   //Resize down to only include good tokens
    }
    output.keep = keep_indeces; //Update keep vector
    return {output};
}
