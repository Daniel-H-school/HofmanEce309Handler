#include "core/sentinel_scanner.h"
#include <string>
#include <cmath>

// class SentinelScanner {
// private:
    std::string sentinel_;  //The sentinel being scanned for
    std::string pending_;   //The chunk of text currently being processed.
                            // Will only be as large as sentinel_.size() - 1
                            // as it only cares when it has found the sentinel
// public:
    SentinelScanner::SentinelScanner(std::string sentinel){
        sentinel_ = sentinel;   //Copies over sentinel it's supposed to be looking for
    }

    //Data type for output type
    struct Out { std::string safe_text; bool sentinel_found; };

    //Returns size of pending for testing
    std::size_t SentinelScanner::pending_size(){
        return pending_.size();
    }

    
    //Feeds next chunk and returns text guaranteed not to be
    //  part of the sentinel, as well as when the sentinel has
    //  been confirmed to have been seen.
    SentinelScanner::Out SentinelScanner::feed(std::string_view chunk){
        if (pending_.size() < (sentinel_.size() - 1)){  //For the very first few chunks until pending_ is the length of sentinel_.size() -1,
            std::string initialCheck = pending_;
            initialCheck.append(chunk);
            std::size_t i = initialCheck.find(sentinel_);  //Checks the chunk without pending_
            if (i == std::string::npos){            //If no sentinel found
                if (pending_.size() + chunk.size() < sentinel_.size()){     //If a whole chunk fits in and pending_ is still smaller than sentinel
                    pending_ += chunk;                                      //Do so
                    return {"", false};                                     //Return nothing yet
                } else {                                                    //Else if a whole chunk would make pending_ longer than sentinel_.size()
                    pending_ = initialCheck.substr(initialCheck.size() - sentinel_.size() + 1, sentinel_.size() - 1);   //pending equals the last sentinel_.size()-1 letters
                    return {initialCheck.substr(0,initialCheck.size() - sentinel_.size() + 1), false};                    //Output first couple of characters
                }
            } else {    //In the very unlikely event that the sentinel was found between the checks.
                        //Shouldn't need to construct an output like below because more than one sentinel won't fit
                return {initialCheck.substr(0,i), true};
            }
        }
        
        std::string toCheck = pending_;     //Creates a new string of pending_ and chunk -> O(N + M)
        toCheck.append(chunk);
        
        std::size_t chunkLen = chunk.size();    //Length of chunk
        std::size_t index = toCheck.find(sentinel_); //Checks for the sentinel text O(M * N)
        
        if (index == std::string::npos){    //If sentinel not found
            pending_ = toCheck.substr(toCheck.size() - (sentinel_.size()-1)); //Takes last letters for next pending
            return {toCheck.substr(0,chunkLen), false}; //Outputs safe text
        } else {
            std::string output = "";                //Creates a string for output to go into
            std::size_t lastIndex = 0;              //Last index for use in constructing output
            while (index != std::string::npos){     //While there are sentinels to find (in case chunk is really big and contains multiple)
                output += toCheck.substr(lastIndex, index - lastIndex);  //Adds to output
                lastIndex = index + sentinel_.size();   //Updates lastIndex to be the next good index
                index = toCheck.find(sentinel_, lastIndex);        //Finds next instance of sentinel (if existant)
            }
            // Once all the sentinels have been found and output has been constructed
            output += toCheck.substr(lastIndex, chunkLen - lastIndex);   //Adds last bit to the output
            pending_ = toCheck.substr(chunkLen); //Set up next pending
            return {output, true};  //Output
        }

    }   

    //Called after stream ends to release data held for checking
    SentinelScanner::Out SentinelScanner::flush(){
        //Since input stream ran out before a sentinel could be confirmed,
        //  whatever is held inside pending_ right now is safe, and
        //  no sentinel was found
        return {pending_, false};
    }
//};
