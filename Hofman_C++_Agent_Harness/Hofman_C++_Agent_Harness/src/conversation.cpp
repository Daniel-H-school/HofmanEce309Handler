#include "message.h"    //Uses message objects
#include <string>       //Uses string data types

//Conversation class
class Conversation {
private:
    Message*    data_ = nullptr;    //The location of the first message
    std::size_t size_ = 0;          //Size of conversation
    std::size_t capacity_ = 0;      //Length of conversation left
public:
    Conversation(int maxTurns = 40){     //Creates a new conversation object by allocating memory
       data_ = new Message[maxTurns];    //Sets data_ to point to the head of the array of messages in conversation
       size_ = maxTurns;                 //Sets the size of the array
       capacity_ = maxTurns;             //Sets the remaining capacity of the array, which equals size because it is empty at construction
    }
    
    //Destructor to clear out dynamic memory
    ~Conversation(){
        delete data_;
    }

    //Copy constructor to deep copy messages into new memory
    Conversation(const Conversation& other){
        delete data_;   //Deletes any junk data about to be copied over
        data_ = new Message[other.size_];     //Allocates new memory for this object
        for (int i = 0; i < other.size_; i++){  //goes through the other conversation message by message
            data_[i] = other.data_[i];  //Copies over message by message
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
    }

    Conversation& operator=(const Conversation& other){
        size_ = other.size_;
        capacity_ = other.capacity_;
        delete data_;   //Deletes current data for new data to copy over
        data_ = new Message[other.size_];       //Allocates enough data
        for (int i = 0; i < other.size_; i++){  //Goes through the other conversation
            data_[i] = other.data_[i];          //Copies over message by message
        }
        return *this;   //Return type
    }

    Conversation(Conversation&& other) noexcept{    //Move operator
        size_ = other.size_;            //Copies over the other size
        capacity_ = other.capacity_;    //Copies over the other capacity
        delete data_;                   //Deletes whatever is currently in this one's data
        data_ = other.data_;    //Makes this data point to the other's already allocated data
        other.data_ = nullptr;  //Makes others data_ point to nothing to prevent it getting deleted unintentionally
    }

    Conversation& operator=(Conversation&& other) noexcept{ //Move assignment operator
        size_ = other.size_;            //Copies over the other size
        capacity_ = other.capacity_;    //Copies over the other capacity
        delete data_;                   //Deletes whatever is currently in this one's data
        data_ = other.data_;    //Makes this data point to the other's already allocated data
        return *this;   //Return type
    }

    void append(Message m){ //Adds this message to the end of the array
        if (capacity_ <= 0){
            //TODO - throw out signal to send turnlimit stop reason
        }
        data_[size_ - capacity_] = m; //Puts message in the next available slot
        capacity_--;    //Decrement capacity
    }

    std::size_t size() const noexcept{  //Returns max size of the conversation
        return size_;
    }

    const Message& at(std::size_t i) const{ //Returns the message at the given address
        return data_[i];
    }

    const Message* begin() const noexcept{  //Returns a pointer to the first message of the conversation
        return data_;
    }

    const Message* begin() const noexcept{  //Returns a pointer to the latest message of the conversation
        return data_ + (size_ - capacity_); //TODO May need to fix this or append. Might point one past capacity for all values but 1
    }
};