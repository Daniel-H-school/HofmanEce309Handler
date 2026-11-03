// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);           //Returns nullptr when trying to find something that doesn't exist
}

TEST(reg_hash_function_tests) {
    ToolRegistry r;
    auto hash1 = r.hash("ABC");
    auto hash2 = r.hash("CBA");
    CHECK(hash1 != hash2);  //Checks that hash function depends on character location
    auto hash3 = r.hash("AB");
    auto hash4 = r.hash("A");
    CHECK(hash1 != hash3 && hash1 != hash4 && hash3 != hash4 && hash4 != hash2);  //Checks that hash function uses all characters and not just the first or last
}

TEST(reg_adding_nullptr) {
    ToolRegistry r;
    REQUIRE(r.add(nullptr) == false);    //Fail gracefully if attempting to add a nullptr
    CHECK(r.size() == 0);       //Make sure that it doesn't add the null tool
    auto sampleTool = std::make_unique<NamedTool>("Sample");
    r.add(std::move(sampleTool));   //Adds a sample to test size operations of nullptr addition with non-empty list
    CHECK(r.size() == 1);   //Check baseline size
    CHECK(r.add(nullptr) == false);
    CHECK(r.size() == 1);   //Check that not actually added
}

TEST(reg_empy_table_operations) {
    ToolRegistry r;
    REQUIRE(r.size() == 0);             //Makes sure hashTable is empty
    CHECK(r.bucket_count() == 8);       //Checks initialization conditions
    CHECK(r.max_probe_length() == 1);   //Checks max probe length is as expected
    CHECK(r.load_factor() == 0.0);      //Checks load factor
    CHECK(r.find("not present") == nullptr);    //Checks that find function runs and returns that it didn't find anything
    std::vector<const Tool*> tools;
    r.for_each([&](const Tool& t) { tools.push_back(&t); });    //Putting in the tools vector a description of each tool in the hash table
    CHECK(tools.empty());   //Since empty hashtable, the tools vector should be empty.
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(reg_scaling_tests){
    ToolRegistry r;
    auto t1 = std::make_unique<NamedTool>("0"); //Tool zero used to check if data actually moves position (i.e. is incorrectly not move()d after reallocation
    Tool* t1ptr = t1.get();
    r.add(std::move(t1));
    for (int i = 1; i < 10000; i++){
        auto toolName = std::to_string(i);
        auto t = std::make_unique<NamedTool>(toolName);   
        r.add(std::move(t));
        CHECK(r.load_factor() <= 0.75);     //Checks that load factor remains under threshold for open indexing after each step
        CHECK(r.max_probe_length() <= 64);  //Checks that max proble length is within reasonable values after each step
    }
    for (int i = 0; i < 10000; i++){
        auto toolName = std::to_string(i);
        auto t = std::make_unique<NamedTool>(toolName);
        auto name = t->name();
        CHECK(r.find(name) != nullptr);   //Checks that each item can be found again
    }
    CHECK(r.find("0") != nullptr);      //Checks that original tool is still in there
    CHECK(t1ptr == r.find("0"));   //Checks that reallocations don't move around memory locations
    CHECK(r.bucket_count() == 16384);   //Checks that bucket count is the nearest power of 2 to 10,000
//    std::cout << "Bucket count: " << r.bucket_count() << std::endl; //Prints out bucket count in case it isn't
    CHECK(r.load_factor() <= 0.75);       //Checks load factor
//    std::cout << "Load factor: " << r.load_factor() << std::endl;  //prints exact load factor
    CHECK(r.max_probe_length() <= 64);  //Checsk max probe length
//    std::cout << "Max Probe Length: " << r.max_probe_length() << std::endl;  //Prints exact max probe length
}

TEST(reg_alternative_hash){
    //Takes hash values for the same 10,000 tool register from above.
    //  Prints the two hashes's max probing value as well as the two values next to each other for a 16,384 length hash table
    ToolRegistry r;
    for (int i = 0; i < 10000; i++){
        r.add(std::make_unique<NamedTool>(std::to_string(i))); //needed to get the hash's buckets to 16,384
    }
    
    std::vector<int> weakHashTable; //Create weak hashtable
    weakHashTable.resize(16384);    //Resize it to the size of the other
    std::size_t longestSearch = 1;              //Longest search variable
    for (std::size_t i = 0; i < 10000; i++){
//        std::cout << "FNV-1a: " <<  r.hash(std::to_string(i)) % r.bucket_count();   //FNV-1a hash from 10000 table
        auto stringI = std::to_string(i);
        std::size_t weakIndex = stringI[0] + stringI.size();    //Weak hash
        std::size_t buckets = 16384;
        std::size_t j = 0;
        while (true){    //Searches around index for open bucket (uses open indexing)
            if (j >= longestSearch) longestSearch++;
            if (weakIndex + j >= buckets){ 
                if (weakHashTable[0 + (weakIndex + j - buckets)] == 0) {
                    weakIndex = (0 + (weakIndex + j - buckets)); 
                    break; //Loops over
                }
            } else if (weakHashTable[weakIndex + j] == 0){   //If open spot
                weakIndex = weakIndex + j;
                break;
            }
            if (weakIndex - j >= buckets){      //Due to size_t being unsigned, will underflow and become much bigger than buckets after going "negative"
                if (weakHashTable[buckets + (weakIndex - j)] == 0) {
                    weakIndex = (buckets + (weakIndex - j));
                    break; //Loops under
                }
            } else if (weakHashTable[weakIndex - j] == 0){
                weakIndex = weakIndex - j;
                break;
            }
            j++;
        }
        weakHashTable[weakIndex] = 1;   //Fills in free spot in table with 1
//        std::cout << "      Weak Hash Function: " << weakIndex << std::endl;
    }
    CHECK(longestSearch > r.max_probe_length());    //Our hash function better be more efficient than this naive one
//    std::cout << "FNV-1a Longest chain: " << r.max_probe_length();
//    std::cout << "      Weak Hash Longest chain: " << longestSearch << std::endl;
//    std::cout << "To examine differences in hash functions more directly, uncomment lines 130 and 158 of this function" << std::endl;

}

TEST(reg_for_each_visits_once){
    ToolRegistry r;
    std::vector<const Tool*> tool_compares;
    //Tools for adding and testing for_each
    for (int i = 0; i < 26; i++){
        auto t = std::make_unique<NamedTool>(std::to_string(i));
        tool_compares.push_back(t.get());
        r.add(std::move(t));
    }

    REQUIRE(r.size() == 26);    //everything added (and not deleted from leaving for scope)
    std::vector<const Tool*> tools;
    r.for_each([&](const Tool& t) {tools.push_back(&t); });   //Should put all tools in vector
    int checkForTwo = 0;    //Tracks if same tool got added to vector twice (i.e. for each happened twice for that tool)
    for (int i = 0; i < 26; i++){
        checkForTwo = 0;    //Resets checkForTWo
        for (std::size_t j = 0; j < tools.size(); j++){
            if (tools[j] == tool_compares[i]){
                checkForTwo++;
            }
        }
        CHECK(checkForTwo == 1);    //The for_each must call each function exactly once
    }
    CHECK(tools.size() == 26);  //Double check for everything being moved over
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_input_output_accepts_table_4_4) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   //Sample file path works
    CHECK(ws.resolve("./notes//todo.txt", &out, &reason)); //Checks that extra /s are taken out by default
    CHECK(out == ws.root() / "notes" / "todo.txt"); //Checks that it simplifies to correct path
    CHECK(reason == "Worked as expected");  //Checks successful output message

    CHECK(ws.resolve(".", &out, &reason));  //Checks that single "." throws no errors
    CHECK(out == ws.root());    //Checks that single "." returns root path

    CHECK(ws.resolve("a/../b",&out,&reason));   //Checks that path can go back a file without erroring (except for going before root)
    CHECK(out == ws.root() / "b");  //Checks that path simplifies correctly

    CHECK(ws.resolve("a/..",&out,&reason)); //Checks that path can go back to root
    CHECK(out == ws.root());                //Checks that path simplifies correctly

    CHECK(ws.resolve("new/dir/file.txt",&out,&reason));   //Checks that it can accept files not yet created
    CHECK(out == ws.root() / "new" / "dir" / "file.txt"); //Checks correct output path

    CHECK(ws.resolve("my..notes.txt",&out,&reason));    //Checks that extra doots doesn't trip up the system
    CHECK(out == ws.root() / "my..notes.txt");          //Checks correct output path

    CHECK(!ws.resolve("",&out,&reason));    //Checks that empty input path is rejected
    CHECK(reason == "empty path");          //Checks correct failure reason

    std::string nullcharString = std::string("notes") + '\0' + "/x";    //Needed because compiler will optimize out \0 by default
    CHECK(!ws.resolve(nullcharString,&out,&reason));   //Checks that path with nullchars in it is rejected before messing string functions up
    CHECK(reason == "unexpected null character in path");   //Checks correct failure reason

    CHECK(!ws.resolve("/etc/passwd",&out,&reason)); //Checks that you can't access based on absolute paths
    CHECK(reason == "only workspace-relative paths allowed");   //Checks correct failure reaason

    CHECK(!ws.resolve("..",&out,&reason));  //Checks that you can't access anything before the root file
    CHECK(reason == "path reaches beyond scope with ../");  //Checks correct failure reason

    CHECK(!ws.resolve("../etc/passwd",&out,&reason));   //Checks that you can't access anything before the root file
    CHECK(reason == "path reaches beyond scope with ../");  //Checks correct failure reason

    CHECK(!ws.resolve("a/../../b",&out,&reason));   //Checks that you can't access anything before the root file
    CHECK(reason == "path reaches beyond scope with ../");  //Checks correct failure reason

    CHECK(!ws.resolve("notes/../../ws/notes/todo.txt",&out,&reason));   //Checks that you can't access anything before the root file, even if you only end up looking for files within it
    CHECK(reason == "path reaches beyond scope with ../");

    CHECK(!ws.resolve("notes/a/b/c/d/../../../e/../../../f/../../",&out,&reason)); //Checks that you can't access anything before the root file even with complicatedly setup ../s
    CHECK(reason == "path reaches beyond scope with ../");  //Checks correct failure reason
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

TEST(calc_input_outputs_table_4_6){
    double result = 0;
    std::string err1;
    std::string* err = &err1;
    
    REQUIRE(eval("10 - 4 - 3", &result, err)); //Checks that repeated subtractions go left to right (10 - 4) - 3 = 3, not 10 - (4 - 3) = 9
    CHECK(result == 3); //Checks correct result
    
    CHECK(eval("100 / 10 / 5", &result, err)); //Checks that repeared divisions go left to right (100 / 10) / 5 = 2, not 100 / (10 / 5) = 50
    CHECK(result == 2); //Checks correct result
    
    CHECK(eval("(2 + 3) * 4", &result, err));  //Checks that basic parenthesis work as expected
    CHECK(result == 20); //Checks correct result
    
    CHECK(eval("2 * (3 + 4) - 5", &result, err));  //Checks order of operations with parenthesis and multiplication on left side
    CHECK(result == 9); //Checks correct result
   
    CHECK(eval("-3 * -3", &result, err));  //Checks that negative numbers are handled correctly
    CHECK(result == 9); //Checks correct result
    
    CHECK(eval("7 / 2", &result, err));    //Checks that output is of double type for decimal answers
    CHECK(result == 3.5);   //Checks correct result
    
    CHECK(!eval("1 / (2 - 2)", &result, err));  //Checks that division by zero is detected while operations are being performed
    CHECK(*err == "division by zero");   //Checks correct failure result
}

TEST(calc_invalid_inputs){
    double result = 0;
    std::string err;
    REQUIRE(!eval("", &result, &err));  //Will not evaluate poorly formatted equations
    CHECK(!eval("( 1 + 4", &result, &err)); //Will not evaluate poorly formatted equations
    CHECK(!eval("()", &result, &err)); //Will not evaluate poorly formatted equations
    CHECK(!eval("7 * 5)", &result, &err));  //Will not evaluate poorly formatted equations
    CHECK(!eval(")", &result, &err));       //Will not evaluate poorly formatted equations
    CHECK(!eval("5 + + 6", &result, &err));  //Will not evaluate poorly formatted equations
    CHECK(!eval("* 5 + 6", &result, &err));  //Will not evaluate poorly formatted equations
    CHECK(!eval("5 + 6 *", &result, &err));  //Will not evaluate poorly formatted equations
}


TEST(calc_large_operations){
    double result = 0;
    std::string err;
    std::string equation1 = "1";
    std::string equation2 = "1.00001";  //To test repeated multiplication, needs to grow very slowly
    std::string equation3 = "20000";
    std::string equation4 = "20000";

    for (int i = 1; i < 20000; i++){    //Sets up very long equation
        equation1 += " + 1";
        equation2 += " * 1.00001";
        equation3 += " / 1.00001";
        equation4 += " - 1";
    }
    REQUIRE(eval(equation1, &result, &err)); //Requires evaluation of the long equation
    CHECK(result == 20000); //Checks that result is as expected

    REQUIRE(eval(equation2, &result, &err)); //Repeated multiplication
    CHECK(result >= 1.221401 && result <= 1.221402); //Checkst that result is as expected (approximately because of rounding errors)

    REQUIRE(eval(equation3, &result, &err));    //Repeated division
    CHECK(result >= 16374.79 && result <= 16374.80);  //Checks that result is as expected (approximately because of rounding errors)

    REQUIRE(eval(equation4, &result, &err));    //Repeated subtraction
    CHECK(result == 1); //Checks that result is as expected
}

TEST(calc_large_parenthetical){
    double result = 0;
    std::string err;
    std::string equation = "2";

    for (int i = 0; i < 5000; i++){ //Sets up long parenthetical equation
        equation += " - ( 2";
    }
    for (int i = 0; i < 5000; i++){
        equation += " ) ";
    }
    REQUIRE(eval(equation,&result,&err));   //Repeated nested parentheticals run without error
    CHECK(result == 2); //Yields correct result (effectively an alternator function going from 2 to 0 an odd number of times)

}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
