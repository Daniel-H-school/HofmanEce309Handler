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
#include <fstream>    

//Used in harness tests. Declared and defined in main for regular build, so must be redefined here
class StdioInput : public InputSource {
public:
    std::string read_line() override {
        std::string line;
        std::getline(std::cin, line);
        eof_ = std::cin.eof();
        return line;
    }
    bool is_eof() const override { return eof_; }

private:
    bool eof_ = false;
};
class StdioOutput : public OutputSink {
public:
    void write(std::string_view text) override {
        std::cout << text << std::flush;
    }
};
const char* role_name(Role role) {
    switch (role) {
        case Role::System: return "system";
        case Role::User: return "user";
        case Role::Assistant: return "assistant";
    }
    return "assistant";
}
void save_transcript(const Conversation& conv, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return;

    bool first = true;
    for (const Message* m = conv.begin(); m != conv.end(); ++m) {
        if (!first) file << "---\n";
        first = false;
        file << "role: " << role_name(m->role()) << "\n";
        file << m->content() << "\n";
    }
}

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
    assert((AddingTo.end()->content() == AddingTo.begin()->content()) && "end() should point to same message as begin with one data point");
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

//Test 3.5: Destructor functionality
// void ConversationDestructor() {
//     Conversation DeleteMe = Conversation();
//     Message DeleteMeToo = Message(Role::User, "Delete");
//     DeleteMe.append(DeleteMeToo);
//     const Message* deleteTest = DeleteMe.begin();
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

//Test 10: Exceeding harness turn limit ends program
void HarnessTurnLimit(){
    HarnessConfig config;
    std::string script_path = "./Scripts/greeting.script";
    config.max_turns = 2;
    auto model = std::make_unique<ScriptedModelClient>(script_path);
    StdioInput in;
    StdioOutput out;

    Harness harness(std::move(model), config);
    StopReason reason = harness.run(in, out);

    // std::cout << reason.detail << std::endl;
    assert((reason.kind == StopReason::Kind::TurnLimit) && "Must stop after turn limit is reached");    //Checks that the conversation ends at the right point
    std::cout << "Turn limit test passed" << std::endl;
}

//Test 11: Reaching sentinel immediately halts loop.
void HarnessSentinelStop(){
    HarnessConfig config;
    std::string script_path = "./Scripts/p2Testing.script";
    config.max_turns = 3;
    auto model = std::make_unique<ScriptedModelClient>(script_path);
    StdioInput in;
    StdioOutput out;

    Harness harness(std::move(model), config);
    StopReason reason = harness.run(in,out);
    assert((reason.kind == StopReason::Kind::Sentinel) && "Must stop at sentinel and terminate loop");  //Checks that the loop stops for the right reason

    Conversation conv = harness.conversation();
//    std::cout << conv.at(1).content() << std::endl;
    assert((conv.end()->content() == "No messages past this point should be reached.<|end_conversation|>") && "Must stop exactly after sentinel is read"); //Checks that the last message in conversation ends right after sentinel
    std::cout << "Sentinel Stop test passed" << std::endl;
}

//Test 12: Transcript playback
void TranscriptPlayback(){
    HarnessConfig config;
    std::string save_path = "./build/transcriptP2Test.txt";     //Test transcript file to playback. Generated through regular operation of program
    config.max_turns = 10;

    StdioInput in1;
    StdioOutput out1;
    auto replayModel = std::make_unique<ReplayModelClient>(save_path);
    Harness harness2(std::move(replayModel), config);

    Conversation replay;
    StopReason stop2 = harness2.run(in1, out1);
    harness2.conversation();
//    std::cout << stop1.detail << std::endl;
//    std::cout << harness.conversation().end()->content() << std::endl;
//    std::cout << harness2.conversation().at(1).content() << std::endl;
    
    assert((stop2.detail == "stop sentinel after 3 turns") && "Must stop for same reason as original transcript");
    assert((harness2.conversation().at(1).content() == "I am doing well, thank you! How can I help you?") && "Return messags must match up");
    assert((harness2.conversation().at(3).content() == "I can definitely do that for you. Anything else?") && "Return messags must match up");
    assert((harness2.conversation().at(5).content() == "Goodbye!<|end_conversation|>") && "Return messags must match up");
    std::cout << "Replay tests passed" << std::endl;
}

int main() {        //Separate main that runs all the tests for project 2
    EmptyConversationBoundsAccessAttempt();
    AddingToConversation();
    ConversationCopyConstructorAllocation();
    // ConversationDestructor();
    ConversationMoveConstructors();
    StressTestingLongConversation();
    ScannerProcessesCleanText();
    ScannerCatchesSentinelAtEveryBoundary();
    ScannerDoesntCatchFalseAlarms();
    ScannerBoundedPendingMemory();
    HarnessTurnLimit();
    HarnessSentinelStop();
    TranscriptPlayback();
    std::cout << "All tests passed" << std::endl;
    return 0;
}
