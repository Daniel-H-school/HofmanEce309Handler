// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"
#include <iostream>

//Constructor function; creates initial buckets
ToolRegistry::ToolRegistry(){
    elements = 0;   //Starts off with no elements
    longestSearch = 1; //initializes for find() function
    buckets = 8;   //Starts off with a relatively small amount of buckets
    hashTable.resize(buckets); //Hash table as the size of the bucket count
} 

//Since the buckets hold unique pointers, the default destructor will get rid of the memory correctly
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 1469598103934665607ULL;   //Offset basis for FNV-1a hash function

    for (unsigned char c : key) {   //For each character in the key
        h ^= c;                 //XOR modifies the lower 8-bits of the hash value based on the current character
        h *= 1099511628211ULL;  //Multiplies by an offset prime which returns the lower 64 bits of the product  
    }

    return static_cast<std::size_t>(h);   // returns the last 64 bits of whatever was computed, cast to the appropriate return type
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {    //Hash table uses open indexing
    if (tool == nullptr) return false;  //If fed a nonexistant tool
    if (find(tool->name()) != nullptr){ //If trying to add a copy of a tool aready in hash table
//        tool->~Tool();   //Destroys this tool copy to free up its memory
        return false;
    } 
    if (((double) elements + 1 / (double) buckets) >= 0.75){ //If this new tool would make the hashtable exceed its threshold for efficient open indexing
        std::vector<std::unique_ptr<Tool>> tempTable; //Temporary hashtable
        tempTable.resize(buckets << 1); //Temptable 2x larger than current table
        for (std::size_t i = 0; i < buckets; i++){  //Indexes through the previous hashtable
            if (hashTable[i] != nullptr){   //If hashtable at that index was equal to nullptr, would not need to copy over
                std::size_t index = hash(hashTable[i]->name()) & (buckets - 1); //Get new hash index of current tool
                tempTable[index] = std::move(hashTable[i]);     //Move over tool pointer to new hashtable
            }
        }
        //After everything copied over
        hashTable = std::move(tempTable);   //Move over the data to active hashtable
        buckets = buckets << 1; //Permanantly doubles buckets
        //Because of vectors, everything should get deleted aftewards


    }
    
    //Once no resizing is needed
    std::size_t index = hash(tool->name()) & (buckets - 1); //Calculate index using hash function
    while (hashTable[index] != nullptr){    //Searches around index for open bucket (uses open indexing)
        longestSearch++;
        if (++index >= buckets) index = 0;  //Iterates back to start after running out of buckets
    }
    hashTable[index] = std::move(tool); //Moves the tool pointer into the hashTable
    elements++;    //Updates elements counter
    return true;   //Return that it has been added
}

//Tries to find a tool by its name's hash value. Searches around according to open indexing
//Returns the raw pointer to the tool -- NOT transfers ownership
Tool* ToolRegistry::find(std::string_view name) const {
    size_t index = (hash(name) & (buckets - 1));
    for (std::size_t i = 0; i < longestSearch; i++){
        if (hashTable[index] != nullptr && hashTable[index]->name().compare(name) == 0){ //If this index has the same name as what we're looking for
            return hashTable[index].get();  //Returns the raw pointer to that tool
        }
        if (name.compare("read_file") == 0){
        }
        //Else when it is not found at that index
        if (++index >= buckets) index = 0;  //Loop around if necessary. If not reiterates with next index
    }
    //If tool was not found within the longest possible search distance, returns nullptr to signify it didn't find it
    return nullptr;
}

//Returns the number of registered tools in this registry
std::size_t ToolRegistry::size() const noexcept {
    return elements;
}

//Calls every function(tool) for every tool in the hashTable
void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    std::size_t found_tools = 0;    //A counter to stop finding elements early if they all have been found already
    for (std::size_t i = 0; i < buckets; i++){  //Goes through each element
        
        if (hashTable[i] != nullptr){
            found_tools++;  //Says it found a tool
            fn(*hashTable[i].get()); //Calls the function for a temporary raw copy of the tool at this address
        }

        if (found_tools >= elements) return; //if found all the elements before the end of the iteration, ends early
    }
}

//Returns number of buckets in the hash table
std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets;
}

//Returns the current load_factor of the hash table
double ToolRegistry::load_factor() const noexcept {
    return (double) elements / (double) buckets;    //Return calculated load factor elements/buckets
}

//Returns the maximum probe length, which has been kept track of via longestSeach var
std::size_t ToolRegistry::max_probe_length() const {
    return longestSearch;
}
