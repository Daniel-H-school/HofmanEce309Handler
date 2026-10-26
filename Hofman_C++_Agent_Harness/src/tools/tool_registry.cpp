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
    std::uint64_t h = 1469598103934665603ULL;   //Offset basis for FNV-1a hash function

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
    if ((((double) elements + 1) / (double) buckets) >= 0.75){ //If this new tool would make the hashtable exceed its threshold for efficient open indexing
        std::vector<std::unique_ptr<Tool>> tempTable; //Temporary hashtable
        tempTable.resize(buckets << 1); //Temptable 2x larger than current table
        longestSearch = 0;  //Resets longest search for resized table
        for (std::size_t i = 0; i < buckets; i++){  //Indexes through the previous hashtable
            if (hashTable[i] != nullptr){   //If hashtable at that index was equal to nullptr, would not need to copy over
                std::size_t index = hash(hashTable[i]->name()) % (buckets << 1); //Get new hash index of current tool
                std::size_t j = 0;
                while (longestSearch < 64){    //Searches around index for open bucket (uses open indexing)
                    if (index + j >= (buckets << 1)){ 
                        if (tempTable[0 + (index + j - (buckets << 1))] == nullptr) {
                            index = (0 + (index + j - (buckets << 1)));
                            break; //Loops over
                        }
                    } else if (tempTable[index + j] == nullptr){   //If open spot
                        index = index + j;
                        break;
                    }
                    if (index - j >= (buckets << 1)){      //Due to size_t being unsigned, will underflow and become much bigger than buckets after going "negative"
                        if (tempTable[(buckets << 1) + (index - j)] == nullptr) {
                            index = ((buckets << 1) + (index - j)); 
                            break; //Loops under
                        }
                    } else if (tempTable[index - j] == nullptr){
                        index = index - j;
                        break;
                    }
                    j++;
                    if (j > longestSearch) longestSearch = j;
                }
                tempTable[index] = std::move(hashTable[i]);
            }
        }
        //After everything copied over
        hashTable = std::move(tempTable);   //Move over the data to active hashtable
        buckets = buckets << 1; //Permanantly doubles buckets
        //Because of vectors, everything should get deleted aftewards
    }
    
    //Once no resizing is needed
//    std::cout << "Adding tool" << std::endl;
    std::size_t index = hash(tool->name()) % (buckets); //Calculate index using hash function
    std::size_t i = 0;
    while (longestSearch < 64){    //Searches around index for open bucket (uses open indexing)
        if (index + i >= buckets){ 
            if (hashTable[0 + (index + i - buckets)] == nullptr) {
                index = (0 + (index + i - buckets)); 
                break; //Loops over
            }
        } else if (hashTable[index + i] == nullptr){   //If open spot
            index = index + i;
            break;
        }
        if (index - i >= buckets){      //Due to size_t being unsigned, will underflow and become much bigger than buckets after going "negative"
            if (hashTable[buckets + (index - i)] == nullptr) {
                index = (buckets + (index - i));
                break; //Loops under
            }
        } else if (hashTable[index - i] == nullptr){
            index = index - i;
            break;
        }
        i++;
        if (i > longestSearch) longestSearch = i;
    }
    hashTable[index] = std::move(tool); //Moves the tool pointer into the hashTable
    elements++;    //Updates elements counter
    return true;   //Return that it has been added
}

//Tries to find a tool by its name's hash value. Searches around according to open indexing
//Returns the raw pointer to the tool -- NOT transfers ownership
Tool* ToolRegistry::find(std::string_view name) const {
    size_t index = (hash(name) % (buckets));
    std::size_t i = 0;
    while (i <= longestSearch){
        //Searches the above indeces
        if (index + i >= buckets){
            if (hashTable[0 + (index + i - buckets)] != nullptr && hashTable[0 + (index + i - buckets)]->name().compare(name) == 0){    //Checks that there is something in that spot, then checks the name
                return hashTable[0 + (index + i - buckets)].get();
            }
        } else {
            if (hashTable[index + i] != nullptr && hashTable[index + i]->name().compare(name) == 0){
                return hashTable[index + i].get();
            }
        }
        //Searches the below indeces
        if (index - i >= buckets){
            if (hashTable[buckets + (index - i)] != nullptr && hashTable[buckets + (index - i)]->name().compare(name)==0){
                return hashTable[buckets + (index - i)].get();
            }
        } else {
            if (hashTable[index - i] != nullptr && hashTable[index - i]->name().compare(name) == 0){
                return hashTable[index - i].get();
            }
        }
        i++;
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
