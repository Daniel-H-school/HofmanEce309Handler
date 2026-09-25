// tests/p2/test_p2.cpp
//
// YOUR test suite goes here. At least 12 assert-based test cases — see
// spec §5 for the required categories and the sample test for the
// expected level of rigor.
//
// This file is a stub so the project builds out of the box; replace the
// body of main() with your own tests.

#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"

#include <memory>   //Unique_ptr
#include <stdexcept> //Throws
#include <cassert>  //Asserts
#include <iostream>   //Printing

//Test 1: Messing around with empty conversations does not result in runtime out-of-bounds errors
void EmptyConversationBoundsAccessAttempt() {
    Conversation EmptyConvo = Conversation();   //Initializes empty pointer
    assert((EmptyConvo.size() == 0) && "Conversation must return 0 size if empty");
    bool e = false;
    try {
        EmptyConvo.at(0);
    } catch (std::out_of_range){
        e = true;
    }
    assert(e && "Array out of bounds not caught");
    assert((EmptyConvo.begin() == nullptr) && "Returns a dummy beginning of conversation");
    assert((EmptyConvo.end() == nullptr) && "returns a dummy end of conversation");
    std::cout << "Empty Conversation Tests Passed" << std::endl;
}

//Test 2: Adding messages to conversations works as expected with begins, ends, and accessing elements
//      By extension, tests whether system messages remain pinned to the front of the conversation,
//      since the harness appends any system messages from the script to the front of the conversation by rule
void AddingToConversation() {
    Conversation AddingTo = Conversation();
    Message Head = Message(Role::System, "Head");       //Simulates a regular system message in a script by being the first to be added to conversation
    Message Middle = Message(Role::User, "Middle");
    Message Tail = Message(Role::User, "Tail");
    AddingTo.append(Head);
    assert((AddingTo.begin()->content() == Head.content()) && "begin() should point to sole message");
    assert((AddingTo.end() == AddingTo.begin()) && "end() should point to same message as begin with one data point");
    AddingTo.append(Middle);
    AddingTo.append(Tail);
    assert((AddingTo.begin()->content() == Head.content()) && "begin() should point to first message");
    assert((AddingTo.end()->content() == Tail.content() && "end should point to most recent message"));
    assert(((AddingTo.at(1).content() == Middle.content()) && (AddingTo.at(1).role() == Middle.role())) && "Index should return message at the index");
    bool e = false;
    try {
        AddingTo.at(4);
        AddingTo.at(-1);
    } catch (std::out_of_range){
        e = true;
    }
    assert(e && "Must catch Array out of bounds errors");
    std::cout << "Basic append tests passed" << std::endl;
}

//Test 3: Copy constructors allocate different areas in memory
void ConversationCopyConstructorAllocation() {
    Conversation ConvoA = Conversation();
    Message MessageA = Message(Role::User, "Find me in Conversation A");
    Message MessageB = Message(Role::User, "Find me in Conversation A");
    ConvoA.append(MessageA);
    Conversation ConvoB = ConvoA;   //Copy Constructor
    assert((ConvoB.begin() != ConvoA.begin()) && "Copy-constructed conversation does not have unique pointers");

    const Message* convoA_MessageA_address = ConvoA.begin();
    const Message* convoB_messageA_address = ConvoB.begin();  //Tracks address of this message
    
    ConvoA.append(MessageB);
    ConvoB = ConvoA;    //Assignment operator
    assert((ConvoB.begin() != ConvoA.begin()) && "Assignment operation does not have unique pointers");
    assert((ConvoB.end()->content() == MessageB.content()) && "Assignment does not actually take in new data");
    
    Conversation ConvoC = Conversation();
    ConvoC.append(MessageB);
    ConvoC.append(MessageA);
    ConvoB = ConvoC;
    assert((ConvoB.begin() != ConvoA.begin()) && "Assignment operator does not delete copied-over memory");

    ConvoB = ConvoB;
    assert((ConvoB.begin() != nullptr) && "Assignment operator deletes memory when copying to self");
    std::cout << "Copy Constructor tests passed" << std::endl;
}

// //Test 3.5: Destructor functionality
// void ConversationDestructor() {
//     Conversation DeleteMe = Conversation();
//     Message DeleteMeToo = Message(Role::User, "Delete");
//     DeleteMe.append(DeleteMeToo);
//     Message* deleteTest = DeleteMe.begin();
//     DeleteMe.~Conversation();
//     assert((deleteTest == nullptr) && "Memory not deleted");
//     std::cout << "Destructor test passed" << std::endl;
// }

//Test 4: Move constructors
void ConversationMoveConstructors() {
    Conversation ConvoA = Conversation();
    Message MessageA = Message(Role::User, "Message A");
    ConvoA.append(MessageA);
    const Message* beginPtr = ConvoA.begin();

    Conversation ConvoB = Conversation();
    ConvoB = std::move(ConvoA);

    assert((ConvoB.begin() == beginPtr) && "After copy, does not point to right side's data");
    assert((ConvoA.begin() == nullptr) && "After copy, does not unassign right side's pointers");
   
    std::cout << "Move constructor tests passed" << std::endl;
}

//Test 5: Long conversations grow by doubling capacity and correctly moving everything over
//       so that at() returns accurate messages
void StressTestingLongConversation() {
    Conversation LongConvo = Conversation();
    Message Head = Message(Role::User, "Head"); //Adds a head message
    LongConvo.append(Head);
    for (int i = 0; i < 998; i++){     //Adds 998 more messages
        LongConvo.append(Message(Role::Assistant, "Junk"));
    }
    Message Middle = Message(Role::User, "Middle"); 
    LongConvo.append(Middle);           //Adds a middle-most message at the 1000th spot
    for (int i = 0; i < 999; i++){  //Adds 999 more messages
        LongConvo.append(Message(Role::Assistant, "Junk2"));
    }
    Message Tail = Message(Role::User, "Tail"); //Adds a tail message at the 2000th spot
    LongConvo.append(Tail);
    assert((LongConvo.size() == 2000) && "Not all messages inserted or counted right");     //Checks if all the messages were added correctly
    assert((LongConvo.capacity() == 2048) && "Not adequately doubling in scale each time");  //Checks if the capaccity checks out. Since it starts at one and doubles each time, it should equal the nearest power of two to 2000, 2048
    assert((LongConvo.at(1000 - 1).content() == Middle.content()) && "at() stopped working");   //Checks for at() returns at large sizes
    assert((LongConvo.begin()->content() == Head.content()) && "Begin not pointing to head");
    assert((LongConvo.end()->content() == Tail.content()) && "End not pointing to most recent message");
    std::cout << "Long Convo tests passed" << std::endl;
}

//Test 6: Sentinel Scanner processes strings with no sentinel correctly
void ScannerProcessesCleanText() {
    const std::string sentinel = "<|end_conversation|>";
    const std::string text = "This is the text testing whether or not the sentinel is able to correctly output the right message if given a fair amount of text with no sentinel.";
    SentinelScanner scanner(sentinel);
    
    std::string output;
    //Splits up message into 10 letter chunks
    for (int i = 0; i < text.size()-1; i+=10){
        auto out = scanner.feed(text.substr(i, 10)); //Takes one tenth of text each time
        output.append(out.safe_text);
    }
    output.append(scanner.flush().safe_text);
    // std::cout << "Input: " << text << std::endl;
    // std::cout << "Output: " << output << std::endl;
    assert((text == output) && "Does not identify and output all text as safe");
    std::cout << "No Sentinel in Text test passed" << std::endl;
}

//Test 7: Sentinel Scanner finds sentinel wherever it is around the chunk boundary
void ScannerCatchesSentinelAtEveryBoundary() {
    const std::string sentinel = "<|end_conversation|>";
    const std::string text = "Goodbye." + sentinel;
    for (std::size_t split = 0; split <= text.size(); ++split){
        SentinelScanner scanner(sentinel);
        auto out1 = scanner.feed(text.substr(0, split));
        auto out2 = scanner.feed(text.substr(split));
        assert((out1.sentinel_found || out2.sentinel_found) && "sentinel must be caught regardless of split point");
        assert((out1.safe_text + out2.safe_text == "Goodbye.") && "Safe text must reconstruct to input without sentinels");
    }
    std::cout << "Sentinel over boundary check passed" << std::endl;
}

//Test 8: Sentinel Scanner does not fire for partial or near sentinel matches
void ScannerDoesntCatchFalseAlarms() {
    const std::string sentinel = "<|end_conversing|>";
    const std::string text = "Goodbye.<|end_conversation|>";
    SentinelScanner scanner(sentinel);
    auto out = scanner.feed(text);
    std::string output = out.safe_text;
    output.append(scanner.flush().safe_text);
    assert((!out.sentinel_found) && "Partial sentinel matches should not be flagged");
    assert((output == text) && "Safe text does not match input text");
    
    std::cout << "Partial sentinel check passed" << std::endl;
}

//Test 9: pending_ never exceeds sentinel.size() -1
void ScannerBoundedPendingMemory(){
    const std::string sentinel = "<|end_conversation|>";
    const std::string text = "This is the text testing whether or not the sentinel is able to correctly output the right message if given a fair amount of text with no sentinel.";
    SentinelScanner scanner(sentinel);
    // std::cout << "sentinel size = " << sentinel.size() << std::endl;
    for (int i = 0; i < text.size(); i+=3){ //Testing with a chunk size of 3
        scanner.feed(text.substr(i,3));
        // std::cout << "pending_size = " << scanner.pending_size() << std::endl;
        assert((scanner.pending_size() <= (sentinel.size())) && "Pending_ should never be longer than sentinel.size() - 1");
    }
    std::cout << "Sentinel Scanner bounds test passed" << std::endl;
}

int main() {        //Separate main that runs all the tests for project 2
    EmptyConversationBoundsAccessAttempt();
    AddingToConversation();
    ConversationCopyConstructorAllocation();
//    ConversationDestructor();
    ConversationMoveConstructors();
    StressTestingLongConversation();
    ScannerProcessesCleanText();
    ScannerCatchesSentinelAtEveryBoundary();
    ScannerDoesntCatchFalseAlarms();
    ScannerBoundedPendingMemory();
    
    return 0;
}
