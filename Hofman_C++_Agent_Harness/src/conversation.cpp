#include "core/conversation.h"    //Uses message objects
#include <string>       //Uses string data types
#include <stdexcept>     //For out of bounds exception throwing

//Conversation class
// class Conversation {
// private:
    Message*    data_ = nullptr;    //The location of the first message
    std::size_t size_ = 0;          //Size of conversation
    std::size_t capacity_ = 0;      //Length of conversation left
// public:
    //Empty conversation constructor. Size = capacity = 0, and no memory is allocated
    Conversation::Conversation(){

    }

    //Destructor to clear out dynamic memory
    Conversation::~Conversation(){
        // if (data_ == nullptr){      //If there isn't any memory allocated or 
        //                             //Owned by this conversation, does nothing
        // } else {
            delete[] data_;           //Otherwise de-allocates it as it leaves
        // }
    }

    //Copy constructor to deep copy messages into new memory
    Conversation::Conversation(const Conversation& other){
        //Since copy constructor will start off with no allocated data, don't need to delete it first
        data_ = new Message[other.capacity_];     //Allocates new memory for this object according to other's capacity
        for (int i = 0; i < other.size_; i++){  //goes through the other conversation message by message
            data_[i] = other.data_[i];  //Copies over message by message
        }
        size_ = other.size_;            //Copies other parameters
        capacity_ = other.capacity_;  
    }

    Conversation& Conversation::operator=(const Conversation& other){
        if (data_ != other.data_ && data_ != nullptr){  //As long as the left side and right side don't share data (should only be possible with c1 = c1) Also checks if data is not allocated and cannot be deleted
            delete[] data_;   //Deletes current data for new data to copy over
        } else {
            return *this;   //In clase it is a c1 = c1, returns early so save on runtime
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = new Message[other.capacity_];   //Allocates enough data
        for (int i = 0; i < other.size_; i++){  //Goes through the other conversation
            data_[i] = other.data_[i];          //Copies over message by message
        }
        return *this;   //Return type
    }

    Conversation::Conversation(Conversation&& other) noexcept{    //Move operator
        size_ = other.size_;            //Copies over the other size
        capacity_ = other.capacity_;    //Copies over the other capacity
        if (data_ != other.data_){      //Checks if both sides are the same object and won't delete the one memory if so
            if (data_ != nullptr) delete[] data_;               //Deletes whatever is currently in this one's data, if anything
            data_ = other.data_;    //Makes this data point to the other's already allocated data
            other.data_ = nullptr;  //Makes others data_ point to nothing to prevent it getting deleted unintentionally
        }
    }

    Conversation& Conversation::operator=(Conversation&& other) noexcept{ //Move assignment operator
        size_ = other.size_;            //Copies over the other size
        capacity_ = other.capacity_;    //Copies over the other capacity
        if (data_ != other.data_){      //In case the two data_ pointers point to the same thing, doesn't delete the only source of data
            delete data_;                   //Deletes whatever is currently in this one's data
            data_ = other.data_;    //Makes this data point to the other's already allocated data
            other.data_ = nullptr;  //Reassigns ownership by removing other.data_'s ownership
            return *this;   //Return type
        }
    }

    void Conversation::append(Message m){ //Adds this message to the end of the array
        if (size_ >= capacity_){                    //If out of room
            if (capacity_ == 0) {
                capacity_ = 1;      //If appending to the uninitialized array, sets it to a value that can be multipliled, as well as allocates memory for data_
                data_ = new Message[capacity_];
                data_[0] = m;
                size_ = 1;
                return;         //Exits early to avoid unnecesary reallocation
            }
            capacity_ = capacity_ * 2;              //Doubles capacity each time it needs to, which keeps ammortized O(1)   
            Message* temp = new Message[capacity_]; //Creates new array to hold previous data with room for twice as much data
            for (int i = 0; i < size_; i++){        //For each element in previous data
                temp[i] = data_[i];                 //Copy over to new array
            }
            delete[] data_;       //Deletes old data just copied over
            data_ = temp;       //Tells data to point to array at temp in heap
            temp = nullptr;     //Shifts ownership so data can get deleted later
        }
        //After allocating new memory (potentially), time to append the next message
        size_++;            //Increments size
        data_[size_ - 1] = m;   //Puts message in new array slot
    }

    std::size_t Conversation::size() const noexcept{  //Returns current size of conversation
        return size_;
    }

    std::size_t Conversation::capacity() const noexcept{  //Returns current capacity of conversation for testing
        return capacity_;
    }

    const Message& Conversation::at(std::size_t i) const{ //Returns the message at the given address
        //Remember to test array_out_of_bounds condition
        if (this->size() == 0 || i >= this->size() || i < 0) { //Array out of bounds
            throw std::out_of_range("Conversation index out of bounds");
        }
        return data_[i];            //Returns data at that address
    }

    const Message* Conversation::begin() const noexcept{  //Returns a pointer to the first message of the conversation
        return data_;
    }

    const Message* Conversation::end()   const noexcept{  //Returns a pointer to the latest message of the conversation
        if (data_ == nullptr) return nullptr;
        return data_ + size_ - 1; //pointer to last message in conversation
    }
//};