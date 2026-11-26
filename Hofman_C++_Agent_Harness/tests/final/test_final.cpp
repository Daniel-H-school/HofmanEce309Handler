// tests/final/test_final.cpp — YOUR TEST SUITE (spec §8).
//
// You need at least 20 tests. The 8 examples below are given and count:
//   Memory  (4): heap, lru, bst, ctx
//   Policy  (2): pol x2
//   Skills  (2): graph, index
// Write at least 12 more of your own, covering the list in spec §8.
// Name tests <area>_<what> with the areas heap, lru, bst, ctx, pol, graph,
// index, so the summary groups them.
//
//   ./build/test_final              run everything
//   ./build/test_final lru_         only tests whose names start with lru_
//   ./build/test_final =lru_evicts_least_recently_used    one test, exactly
//   ./build/test_final --list       list test names
//
// If a sanitizer error (ASan/UBSan) aborts the run, the tests after it do
// not run: rerun the failing test alone with =its_name and read the FIRST
// stack frame that is in your own file.
//
// CHECK_EQ(a, b) prints both values when it fails (vectors too), so prefer
// it over CHECK(a == b).
//
// Helpers you can use (tests/test_helpers.h): TempDir, write_text,
// write_skill, FixedPolicy, ScriptedConfirmer, LinesInput, StringOutput,
// parse_ok, args1. ManualClock is in policy/policy.h.

#include "../mini_test.h"
#include "../test_helpers.h"

#include "memory/context_builder.h"
#include "memory/memory_index.h"
#include "memory/memory_store.h"
#include "policy/policies.h"
#include "policy/policy_chain.h"
#include "skills/dep_graph.h"
#include "skills/skill_catalog.h"
#include "skills/skill_index.h"
#include "util/lru_cache.h"
#include "util/min_heap.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <random>

// =========================================================================
// Memory — 4 examples
// =========================================================================

//Tests functionality with rare empty heap cases. Makes sure program doesn't crash
TEST(heap_empty_operations){
    MinHeap<int> h;
    CHECK_EQ(h.size(),std::size_t{0});  //Checks that size is zero
    CHECK_EQ(h.empty(), true);          //Checks taht the heap recognizes itself as empty
    int errorCatch = 0;
    try {
        h.top();    //Checks that you can't access elements of an empty heap
    } catch (const std::exception& e) {
        std::string error = e.what();
        if (error.compare("Attempted Access of Empty Minheap") == 0)       
        errorCatch++;   //Increments a flag variable
    }
    try {
        h.pop();
    } catch (const std::exception& e){
        std::string error = e.what();
        if (error.compare("Attempted Access of Empty Minheap") == 0) 
        errorCatch += 2;
    }
    CHECK_EQ(errorCatch, 3);    //Checks that both accessing out of bounds errors were caught
}
// Example 1 (heap): items come back smallest first, duplicates included.
TEST(heap_pops_in_order) {
    MinHeap<int> h;
    for (int x : {5, 1, 4, 1, 3}) h.push(x);
    CHECK_EQ(h.size(), std::size_t{5});
    CHECK_EQ(h.top(), 1);
    std::vector<int> got;
    while (!h.empty()) got.push_back(h.pop());
    CHECK_EQ(got, (std::vector<int>{1, 1, 3, 4, 5}));
}

TEST(heap_clear_and_manual_empty){
    MinHeap<int> h;
    for (int i = 0; i < 100; i++){  //Adds a hundred things to the heap
        h.push(i);
    }

    while (!h.empty()){ //Pops everything to test emptiness
        h.pop();
    }
    CHECK_EQ(h.empty(), true);  //Double checks that it is empty
    int errFlag = 0;
    try {
        h.top();
    } catch (const std::exception& e){
        std::string error = e.what();
        if (error.compare("Attempted Access of Empty Minheap") == 0)
        errFlag++;
    }
    CHECK_EQ(h.size(), std::size_t{0});  //Checks the size function
    CHECK_EQ(errFlag, 1);   //Checks that an emptied still counts as empty, as before

    for (int i = 0; i < 100; i++){  //Pushes another 100 objects to the heap
        h.push(i);
    }
    h.clear();  //Tries the clear function
    CHECK_EQ(h.size(), std::size_t{0});      //Checks that size is 0
    CHECK_EQ(h.empty(), true);  //Checks that heap is empty
}

//Boundary conditions for top_k function
TEST(heap_top_k_empty_edge_cases){
    std::vector<int> v;
    std::vector<int> empty = top_k(v,10);
    std::vector<int> emptyCheck = {};
    CHECK_EQ(empty, emptyCheck);
    empty = top_k(v,0);
    CHECK_EQ(empty, emptyCheck);
    for (int i = 0; i < 10; i++){
        v.push_back(i);
    }
    empty = top_k(v,0);
    CHECK_EQ(empty, emptyCheck);
}

//Expected top_k behavior
TEST(heap_top_k_Expected_small_result){
    std::vector<int> v;
    for (int x : {5,16,9,17,351,32,1512,523,61,32,4,61,9,9,0,1231}) {
        v.push_back(x); 
    }
    std::vector<int> smallCheck = top_k(v,5);
    std::vector<int> largeCheck = top_k(v,12);
    std::vector<int> overSizedCheck = top_k(v,100);
    CHECK_EQ(smallCheck, (std::vector<int>{1512,1231,523,351,61}));
    CHECK_EQ(largeCheck, (std::vector<int>{1512,1231,523,351,61,61,32,32,17,16,9,9}));
    CHECK_EQ(overSizedCheck, (std::vector<int>{1512,1231,523,351,61,61,32,32,17,16,9,9,9,5,4,0}));
}

//Comparison with built-in sort function in time and use
TEST(heap_large_scale_sort_comparison){
    std::vector<int> v;
    for (int i = 0; i < 10000; i++){    //10,000 entries
        v.push_back(std::rand());   //Inserts a random number to the end
    }
    SteadyClock clock;  //Creates a steady clock for printing out time lengths
    double startK = clock.now_seconds();
    std::vector<int> topTenK = top_k(v,10);    //Takes top 10 items
    double endK = clock.now_seconds();
    double kTime = endK - startK;

    double startSort = clock.now_seconds();
    std::sort(v.begin(), v.end());
    double endSort = clock.now_seconds();
    double sortTime = endSort - startSort;

    std::cout << "std::sort time: " << sortTime << "s" << std::endl;
    std::cout << "top_k time: " << kTime << "s" << std::endl;
    CHECK_EQ(kTime < sortTime, true);   //Checks that top_k function is faster than sort
    
    std::vector<int> topTen;
    for (std::size_t i = v.size() - 1; i >= v.size() - 10; i--){
        topTen.push_back(v[i]); //Pushes back the top 10 indeces for the sorted array
    }

    CHECK_EQ(topTen, topTenK);  //Checks that both sorts correctly sort the data
}

TEST(lru_empty_cases){
    LruCache<std::string, int> c(0);
    c.put("DON'T PUT ME", 12);
    CHECK_EQ(c.contains("DON'T PUT ME"), false); //Should not contain this value since capacity is zero
    CHECK_EQ(c.size(), std::size_t{0});      //Should be empty
    CHECK_EQ(c.capacity(), std::size_t{0});  //Should have no capacity
    CHECK_EQ(c.get("can't find"), nullptr); //Should safely return nullptr before looking
    CHECK_EQ(c.erase(std::string{"n"}), false);  //Shouldn't be able to erase nonexistant data
    CHECK_EQ(c.keys_mru_to_lru(), std::vector<std::string>{});    //Should return an empty vector if using this function
    c.~LruCache();   //Destructor should run smoothly

    LruCache<std::string, int> almostEmpty(1);  //Capacity of one cache to test remove cases with one capacity
    almostEmpty.put("Put Me!", 1);
    CHECK_EQ(almostEmpty.erase("Put Me!"), true);  //Should successfully erase this
    CHECK_EQ(almostEmpty.size(), std::size_t{0});      //Size should now be 0
    CHECK_EQ(almostEmpty.keys_mru_to_lru(), std::vector<std::string>{});    //Should return an empty vector if using this function
}

TEST(lru_put_on_existing_key){
    LruCache<std::string, int> c(4);
    c.put("a",1);
    c.put("b",2);
    c.put("b",3);
    c.put("b",4);
    REQUIRE(c.size()==2);   //Checks proper size
    CHECK_EQ(*c.get("b"),4);    //Checks that the value gets overridden
    CHECK_EQ(c.stats().evictions,std::size_t{0});    //Nothing should be evicted
    CHECK_EQ(c.stats().hits,std::size_t{1});         //Should only register one hit with the get function
}

TEST(lru_stats){
    LruCache<std::string, int> c(5);
    c.put("a",1); c.put("b",2); c.put("c",3); c.put("d",4); c.put("e",5);   //Put 5 keys in the cache
    c.put("NEW",10);    //Should evict the least used ("a")
    c.put("b",100);     //Should not evict the duplicate key
    c.get("a");         //Should miss, as a has been evicted
    c.get("NEW");       //Should hit
    c.put("NEW2",90);   //Should evict the new least used ("c")
    c.get("c");         //Should miss
    c.get("d");         //Should hit
    CHECK_EQ(c.stats().evictions, std::size_t{2});
    CHECK_EQ(c.stats().misses,std::size_t{2});
    CHECK_EQ(c.stats().hits,std::size_t{2});
}

// Example 2 (lru): the least recently USED entry is evicted, and get()
// counts as a use.
TEST(lru_evicts_least_recently_used) {
    LruCache<std::string, int> c(2);
    c.put("a", 1);
    c.put("b", 2);
    REQUIRE(c.get("a") != nullptr);   // "a" is now the most recent
    c.put("c", 3);                    // full: "b" must go
    CHECK(c.contains("a"));
    CHECK(!c.contains("b"));
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<std::string>{"c", "a"}));
    CHECK_EQ(c.stats().evictions, std::size_t{1});
}

TEST(lru_get_refreshes_recency){
    LruCache<std::string, int> c(10);
    for (int i = 0; i < 10; i++){           //Puts ten items in the cache, a-j
        c.put(std::to_string(i),i);
    }
    CHECK_EQ(c.keys_mru_to_lru(),(std::vector<std::string>{"9","8","7","6","5","4","3","2","1","0"}));  //Checks that insertion puts at most recently used position
    c.get("1");
    c.get("3");
    c.get("4");
    CHECK_EQ(c.keys_mru_to_lru(),(std::vector<std::string>{"4","3","1","9","8","7","6","5","2","0"}));  //Checks proper reordering
}

TEST(lru_erasing){
    LruCache<std::string, int> c(5);
    c.put("tail",1); c.put("b",2); c.put("c",3); c.put("d",4); c.put("head",5);   //Put 5 keys in the cache
    c.erase("head");    //Erases head
    REQUIRE(c.size() == 4);
    CHECK_EQ(c.keys_mru_to_lru(),(std::vector<std::string>{"d","c","b","tail"}));
    c.erase("tail");    //Erases tail
    REQUIRE(c.size() == 3);
    CHECK_EQ(c.keys_mru_to_lru(),(std::vector<std::string>{"d","c","b"}));
    c.erase("c");       //Erases something in the middle
    REQUIRE(c.size() == 2);
    CHECK_EQ(c.keys_mru_to_lru(),(std::vector<std::string>{"d","b"}));
    //Erasing near-empty values covered earlier
}

//Test for niche empty cases
TEST(bst_empty_cases){
    MemoryIndex t;
    CHECK_EQ(t.find(0),nullptr);    //Checks return type of find with empty bst
    CHECK_EQ(t.remove(0),false);    //Checks correct return type of remove with empty bst
    CHECK_EQ(t.size(), std::size_t{0});          //Checks size = 0
    CHECK_EQ(t.height(), std::size_t{0});        //Checks max height = 0
    std::vector<std::uint64_t> ids;
    t.for_each_in_order([&](const MemoryRecord& r) {ids.push_back(r.id);});
    t.clear();                                                  //Checks that clear function doesn't crash
}

//Tests removals of leafs (except for root)
TEST(bst_remove_leaf){
    MemoryIndex t;
    for (std::uint64_t id : {10,5,15,12,17}) REQUIRE(t.insert(MemoryRecord{id, "m" + std::to_string(id)})); //Insert several nodes
    t.remove(5);
    //vector of ids for comparison
    std::vector<std::uint64_t> ids;
    t.for_each_in_order([&](const MemoryRecord& r) {ids.push_back(r.id);});
    CHECK_EQ(ids, (std::vector<std::uint64_t>{10,12,15,17}));   //Check removal of leaf
    t.remove(12);   
    ids.clear();
    t.for_each_in_order([&](const MemoryRecord& r) {ids.push_back(r.id);});
    CHECK_EQ(ids, (std::vector<std::uint64_t>{10,15,17}));      //Check removal of leaf
    CHECK_EQ(t.size(),std::size_t{3});   //Check size after removals
}

//Tests removal logic for one child cases, excluding root
TEST(bst_remove_one_child){
    MemoryIndex t;
    for (std::uint64_t id : {25,12,8,6,18,13,30,26,27,32,39}) REQUIRE(t.insert(MemoryRecord{id, "m" + std::to_string(id)}));    //Inserts several nodes
    //Creates 4 separate one child cases:
    //  8,6 is ll
    //  18,13 is rl
    //  26,27 is lr
    //  32,39 is rr
    t.remove(8);
    t.remove(18);
    t.remove(26);
    t.remove(32);
    std::vector<std::uint64_t> ids;
    t.for_each_in_order([&](const MemoryRecord& r) {ids.push_back(r.id);});
    CHECK_EQ(ids, (std::vector<std::uint64_t>{6,12,13,25,27,30,39}));   //Check shape after removals
    CHECK_EQ(t.size(),std::size_t{7});   //Check size after removals
}

// Example 3 (bst): removing a node with two children keeps the tree sorted,
// keeps every other record, and lowers size() by exactly one.
TEST(bst_remove_two_children) {
    MemoryIndex t;
    for (std::uint64_t id : {50, 30, 70, 60, 80}) REQUIRE(t.insert(MemoryRecord{id, "m" + std::to_string(id)}));
    REQUIRE(t.remove(70));
    std::vector<std::uint64_t> ids;
    t.for_each_in_order([&](const MemoryRecord& r) { ids.push_back(r.id); });
    CHECK_EQ(ids, (std::vector<std::uint64_t>{30, 50, 60, 80}));
    CHECK(t.find(70) == nullptr);
    REQUIRE(t.find(80) != nullptr);
    CHECK_EQ(t.find(80)->text, std::string("m80"));
    CHECK_EQ(t.size(), std::size_t{4});
}

//Tests removal logic for removing the root node, both with and without a deeper tree
TEST(bst_remove_root){
    MemoryIndex t;
    t.insert(MemoryRecord{1,"1"});  //Inserts a root and nothing else
    t.remove(1);    //Remove the sole node which is the root
    std::vector<std::uint64_t> ids;
    t.for_each_in_order([&](const MemoryRecord& r) { ids.push_back(r.id); });
    CHECK_EQ(ids, (std::vector<std::uint64_t>{}));  //Checks that ids is empty
    CHECK_EQ(t.size(), std::size_t{0});  //Checks that size is empty

    //Inserts nodes for a left one child case at root
    for (std::uint64_t id : {50, 30, 20, 33}) REQUIRE(t.insert(MemoryRecord{id, "m" + std::to_string(id)}));
    t.remove(50);
    ids.clear();
    t.for_each_in_order([&](const MemoryRecord& r) { ids.push_back(r.id); });
    CHECK_EQ(ids, (std::vector<std::uint64_t>{20,30,33}));  //Check root was removed correctly
    CHECK_EQ(t.size(), std::size_t{3});  //Check size

    //Inserts nodes for a right one child case at root
    t.clear();
    for (std::uint64_t id : {50, 60, 55, 65}) REQUIRE(t.insert(MemoryRecord{id, "m" + std::to_string(id)}));
    t.remove(50);
    ids.clear();
    t.for_each_in_order([&](const MemoryRecord& r) { ids.push_back(r.id); });
    CHECK_EQ(ids, (std::vector<std::uint64_t>{55,60,65}));  //Check root was removed correctly
    CHECK_EQ(t.size(), std::size_t{3});  //Checks size

    //Inserts nodes for a two child case with successor logic
    t.clear();
    for (std::uint64_t id : {50, 40, 60, 55, 65}) REQUIRE(t.insert(MemoryRecord{id, "m" + std::to_string(id)}));
    t.remove(50);
    ids.clear();
    t.for_each_in_order([&](const MemoryRecord& r) { ids.push_back(r.id); });
    CHECK_EQ(ids, (std::vector<std::uint64_t>{40,55,60,65}));   //Check root was removed correctly
    CHECK_EQ(t.size(), std::size_t{4});
}

//Tests height with known resultss
TEST(bst_height_determined){
    MemoryIndex t;
    for (std::uint64_t id : {25,12,6,9,8,7,30,27,32,31,45,44,50}) REQUIRE(t.insert(MemoryRecord{id, "m" + std::to_string(id)}));
    //Correct height from 25->12->6->9->8->7 = 6
    CHECK_EQ(t.height(),std::size_t{6}); //Checks correct height

    t.remove(9);    //Removes two nodes from left side to make right side taller
    t.remove(8);
    CHECK_EQ(t.height(),std::size_t{5}); //Checks correct height from right path
}

//Prints height for random and unbalanced trees
TEST(bst_height_comparison){
    MemoryIndex t;
    MemoryIndex t2;
    for (std::uint64_t i = 0; i < 1000; i++){
        REQUIRE(t.insert(MemoryRecord{i, std::to_string(i)}));  //Insert sequentially
        REQUIRE(t2.insert(MemoryRecord{(std::uint64_t) std::rand(), std::to_string(i)}));    //Insert random values
    }    
    CHECK_EQ(t.size(), std::size_t{1000});  //Checks both sizes are 1000
    CHECK_EQ(t.size(), std::size_t{1000});  
    std::cout << "Height unbalanced: " << t.height() << std::endl;
    std::cout << "Height random: " << t2.height() << std::endl;
}

//Checks memory select_context for empty context vectors, empty budgets, and empty keeps
TEST(ctx_empty_cases) {
    std::vector<ContextItem> emptyVector;
    ContextPlan p = select_context(emptyVector,10,10);
    CHECK_EQ(p.keep, std::vector<std::size_t>{});       //Checks empty vector
    CHECK_EQ(p.total_tokens, std::size_t{0});           //Checks lack of total tokens
    CHECK_EQ(p.over_budget, false);                     //Checks that isn't over budget

    emptyVector.push_back({Role::Assistant,10,false});
    //Empty vector now has one item
    ContextPlan p2 = select_context(emptyVector,0,0);   //Since budget is zero, and nothing is marked to be kept, should be an empty context plan
    CHECK_EQ(p.keep, std::vector<std::size_t>{});       //Checks that nothing is kept
    CHECK_EQ(p.total_tokens, std::size_t{0});           //Checks lack of total tokens
    CHECK_EQ(p.over_budget, false);                     //Checks that it stays under budget
}

// Example 4 (ctx): the worked example in spec §4.4.
TEST(ctx_spec_example) {
    std::vector<ContextItem> items{
        {Role::System, 10, false},     // 0  protected
        {Role::User, 10, false},       // 1
        {Role::Assistant, 10, false},  // 2  dropped 3rd (oldest assistant)
        {Role::Tool, 10, false},       // 3  dropped 1st
        {Role::Assistant, 10, false},  // 4
        {Role::Tool, 10, false},       // 5  dropped 2nd
        {Role::User, 10, false},       // 6  recent: protected
        {Role::Assistant, 10, false}}; // 7  recent: protected
    ContextPlan p = select_context(items, 50, 2);
    CHECK_EQ(p.keep, (std::vector<std::size_t>{0, 1, 4, 6, 7}));
    CHECK_EQ(p.total_tokens, std::size_t{50});
    CHECK(!p.over_budget);
}

//Test for a few shortcuts regarding all tokens being under budget and all tokens being considered recent
TEST(ctx_shortcuts){
    std::vector<ContextItem> items {
        {Role::System, 10, false},
        {Role::User, 10, false},
        {Role::Assistant, 10, false}};
    ContextPlan p1 = select_context(items,40,0);    //When total budget is greater than total tokens
    ContextPlan p2 = select_context(items,30,0);    //When total budget is equal to total tokens
    ContextPlan p3 = select_context(items,0,4);     //When more than all items are considered recent
    ContextPlan p4 = select_context(items,0,3);     //When exactly all items are considered recent 
    ContextPlan p5 = select_context(items,30,3);    //When multiple cases are true
    
    CHECK_EQ(p1.keep, (std::vector<std::size_t>{0,1,2}));   //Checks that all vectors successfully added everything
    CHECK_EQ(p2.keep, (std::vector<std::size_t>{0,1,2}));
    CHECK_EQ(p3.keep, (std::vector<std::size_t>{0,1,2}));
    CHECK_EQ(p4.keep, (std::vector<std::size_t>{0,1,2}));
    CHECK_EQ(p5.keep, (std::vector<std::size_t>{0,1,2}));

    CHECK_EQ(p1.total_tokens, std::size_t{30}); //Checks all of the sizes
    CHECK_EQ(p2.total_tokens, std::size_t{30});
    CHECK_EQ(p3.total_tokens, std::size_t{30});
    CHECK_EQ(p4.total_tokens, std::size_t{30});
    CHECK_EQ(p5.total_tokens, std::size_t{30});

    CHECK(!p1.over_budget); //Shouldn't be over budget
    CHECK(!p2.over_budget); //Shouldn't be over budget
    CHECK(p3.over_budget);  //Should be over budget
    CHECK(p4.over_budget);  //Should be over budget
    CHECK(!p5.over_budget); //Shouldn't be over budget
}

//Tests ctx budget works with multiple token sizes
TEST(ctx_budget_multiple_tokens){
    std::vector<ContextItem> items {
        {Role::System, 5, false},
        {Role::User, 10, false},
        {Role::Assistant, 15, false},
        {Role::User, 20, false},
        {Role::Assistant, 25, false}    //Total tokens = 75
    };
    ContextPlan p1 = select_context(items,70,10);   //Should select all items and be over budget
    ContextPlan p2 = select_context(items,75,10);   //Should select all items and be exactly on budget
    ContextPlan p3 = select_context(items,50,1);    //Should be able to stay under budget through selection process
    ContextPlan p4 = select_context(items,49,1);    //Should be able to stay under budget by making a sacrifice in what to take

    CHECK(p1.over_budget);      //Checks that p1 is correctly over budget
    CHECK(!p2.over_budget);     //Checks that p2 is correctly under budget
    CHECK(!p3.over_budget);
    CHECK(!p4.over_budget);

    CHECK_EQ(p3.total_tokens, std::size_t{50}); //75 (total) - 15 = 60 - 10 = 50 
    CHECK_EQ(p4.total_tokens, std::size_t{30}); //75 (total) - 15 = 60 - 10 = 50 - 20 = 30
}

//Tests ctx keeps expected order
TEST(ctx_survival_order){
    std::vector<ContextItem> items {
        {Role::System, 10, false},      //System message -> kept
        {Role::User, 10, false},        //Less recent than other user -> lost
        {Role::Assistant, 10, false},   //Next least important message ->lost
        {Role::User, 10, false},        //More recent -> kept
        {Role::Assistant, 10, true},    //Pinned message -> kept
        {Role::Tool, 10, false},        //Least important message -> lost
        {Role::User,10,false},          //Most recent 2 -> kept
        {Role::Assistant,10,false}      //Most recent 2 -> kept
    };
    ContextPlan p = select_context(items,50,2); //Should select according to top logic
    CHECK_EQ(p.keep, (std::vector<std::size_t>{0,3,4,6,7}));  //Keeps as expected
    CHECK_EQ(p.total_tokens, std::size_t{50});  //Should have this many tokens
    CHECK(!p.over_budget);  //Should be on budget
}

//Tests Over budget for complicated removal setup
TEST(ctx_over_budget){
    std::vector<ContextItem> items {
        {Role::System, 10, false},      //System message -> kept
        {Role::User, 10, false},        //Less recent than other user -> lost
        {Role::Assistant, 10, false},   //Next least important message ->lost
        {Role::User, 10, false},        //More recent -> kept
        {Role::Assistant, 10, true},    //Pinned message -> kept
        {Role::Tool, 10, false},        //Least important message -> lost
        {Role::User,10,false},          //Most recent 2 -> kept
        {Role::Assistant,10,false}      //Most recent 2 -> kept
    };    
    ContextPlan p1 = select_context(items, 30, 3);  //Should overburden the budget with too much recency
    ContextPlan p2 = select_context(items, 10, 0);  //Should overburden the budget with system and pinned messages

    CHECK_EQ(p1.keep, (std::vector<std::size_t>{0,4,5,6,7}));   //Should keep all these
    CHECK_EQ(p1.total_tokens, std::size_t{50}); //Checks correct number of tokens
    CHECK(p1.over_budget);  //Should be over budget

    CHECK_EQ(p2.keep, (std::vector<std::size_t>{0,4}));    //Should only keep system and pinned messages
    CHECK_EQ(p2.total_tokens, std::size_t{20});
    CHECK(p2.over_budget);  //Should be over budget
}

// =========================================================================
// Policy — 2 examples
// =========================================================================

//Tests empty cases for the four policy types
TEST(pol_empty_cases){
    AllowlistPolicy emptyA(std::vector<std::string>{});       //Empty allowlist policy
    ArgumentPatternPolicy emptyP(std::vector<std::string>{}); //Empty argument pattern policy
    ConfirmPolicy emptyC(std::vector<std::string>{});         //Empty confirm policy
    ManualClock clock(0);           //Manual clock initialized to zero for rate limit policy
    RateLimitPolicy emptyCl(0,0, clock);    //Empty (as empty as it can get) ratelimit policy

    CommandRequest sampleRequest{"Allow me", std::vector<std::string>{"Arga","Argb"}, "id=10"};
    CommandRequest emptyRequest{"",std::vector<std::string>{},""};
    CHECK_EQ(emptyA.evaluate(sampleRequest).kind, Decision::Kind::Deny); //Nothing on allow list: deny
    CHECK_EQ(emptyA.evaluate(emptyRequest).kind, Decision::Kind::Deny);  //Nothing on allow list: deny

    CHECK_EQ(emptyP.evaluate(sampleRequest).kind, Decision::Kind::Allow);   //No patterns in deny list: allow
    CHECK_EQ(emptyP.evaluate(emptyRequest).kind, Decision::Kind::Allow);    //No patterns in deny list: allow

    CHECK_EQ(emptyC.evaluate(sampleRequest).kind, Decision::Kind::Allow);   //No programs on request list: Allow
    CHECK_EQ(emptyC.evaluate(emptyRequest).kind, Decision::Kind::Allow);    //No programs on request list: Allow

    CHECK_EQ(emptyCl.evaluate(sampleRequest).kind, Decision::Kind::Deny);   //bandwidth is zero: Denies every request
    CHECK_EQ(emptyCl.evaluate(emptyRequest).kind, Decision::Kind::Deny);    //  
}

//Allow list allow/deny
TEST(pol_allow_list_cases){
    AllowlistPolicy a(std::vector<std::string>{"Allow Me", "2nd","Third"});
    CommandRequest denyRequest{"Deny Me",std::vector<std::string>{"Args"},"id=0"};
    CommandRequest allowRequest{"Allow Me", std::vector<std::string>{"Args"},"id=1"};
    CommandRequest allowDownTheLineRequest{"Third",std::vector<std::string>{"Args"},"id=2"};

    CHECK_EQ(a.evaluate(denyRequest).kind, Decision::Kind::Deny);
    CHECK_EQ(a.evaluate(allowRequest).kind, Decision::Kind::Allow);
    CHECK_EQ(a.evaluate(allowDownTheLineRequest).kind, Decision::Kind::Allow);
}

//ArgumentPattern Blacklist allow/deny
TEST(pol_argument_pattern_poliicy_cases){
    ArgumentPatternPolicy p(std::vector<std::string>{"deny","don't do","123 xyz"});
    CommandRequest allowRequest{"Allow Me", std::vector<std::string>{"allow", "123abc"}, "id=0"};
    CommandRequest deny1stArgs{"Deny Me", std::vector<std::string>{"deny","This ok"}, "id=1"};
    CommandRequest denyLaterArgs{"Deny Me 2", std::vector<std::string>{"fine", "alright", "don't do"}, "id=2"};
    CommandRequest denySplitNames{"Deny Me 123", std::vector<std::string>{"xyz", "other arg"}, "id=3"};

    CHECK_EQ(p.evaluate(allowRequest).kind, Decision::Kind::Allow);
    CHECK_EQ(p.evaluate(deny1stArgs).kind, Decision::Kind::Deny);
    CHECK_EQ(p.evaluate(denyLaterArgs).kind, Decision::Kind::Deny);
    CHECK_EQ(p.evaluate(denySplitNames).kind, Decision::Kind::Deny);
}

//Confirm Policy Ask/Allow
TEST(pol_confirm_policy_cases){
    ConfirmPolicy c(std::vector<std::string>{"Ask Me!", "number 2", "3"});
    CommandRequest allowRequest{"Allow Me", std::vector<std::string>{"args"},"id=0"};
    CommandRequest askRequest{"Ask Me!", std::vector<std::string>{"args"},"id=1"};
    CommandRequest askLater{"3", std::vector<std::string>{"args"},"id=2"};

    CHECK_EQ(c.evaluate(allowRequest).kind, Decision::Kind::Allow);
    CHECK_EQ(c.evaluate(askRequest).kind, Decision::Kind::Ask);
    CHECK_EQ(c.evaluate(askLater).kind, Decision::Kind::Ask);
}

// Example 5 (pol): the sliding window. A request exactly `window` seconds
// after an allowed one no longer counts it.
TEST(pol_rate_limit_window_boundary) {
    ManualClock clock(100);
    RateLimitPolicy p(2, 10.0, clock);   // at most 2 per 10 s
    CommandRequest req;
    req.program = "ls";
    CHECK(p.evaluate(req).kind == Decision::Kind::Allow);   // t = 100
    clock.advance(1);
    CHECK(p.evaluate(req).kind == Decision::Kind::Allow);   // t = 101
    clock.advance(1);
    CHECK(p.evaluate(req).kind == Decision::Kind::Deny);    // t = 102: two in the window
    clock.advance(8);
    CHECK(p.evaluate(req).kind == Decision::Kind::Allow);   // t = 110: 100 has left (110 - 100 = 10)
    CHECK(p.tracked() <= 2u);
}

TEST(pol_chain_empty_cases){
    PolicyChain emptyChain;
    CommandRequest sampleRequest{"Sample",std::vector<std::string>{"Example"},"id=0"};
    CommandRequest emptyRequest;

    CHECK(!emptyChain.add(nullptr));    //Deny addition of nullptr child that could gum up the works

    CHECK_EQ(emptyChain.evaluate(sampleRequest).kind, Decision::Kind::Deny);    //Deny evaluation with no children
    CHECK_EQ(emptyChain.evaluate(emptyRequest).kind, Decision::Kind::Deny);
    CHECK_EQ(emptyChain.evaluate(sampleRequest).policy, "chain");       //Checks correct policy for denial
}

//Being denied on one policy denies entire request, even if allowed on earlier policy
TEST(pol_chain_precedence){
    PolicyChain pChain;
    pChain.add(std::make_unique<AllowlistPolicy>(std::vector<std::string>{"Allowed Here"}));
    pChain.add(std::make_unique<ArgumentPatternPolicy>(std::vector<std::string>{"Not", "Allowed Here"}));
    CommandRequest allowDeny{"Allowed Here",std::vector<std::string>{"Not","Allowed Here"}, "id=0"};
    Decision d = pChain.evaluate(allowDeny);
    CHECK_EQ(d.kind,Decision::Kind::Deny);  //Checks that it denies this request
    CHECK_EQ(d.policy, "patterns");     //Checks that the right thing denies it
}

// Example 6 (pol): first Deny wins and later policies are not asked; with no
// Deny, the FIRST Ask is returned unchanged.
TEST(pol_chain_first_deny_stops_first_ask_wins) {
    auto ask1 = std::make_unique<FixedPolicy>(Decision::Kind::Ask, "ask1");
    auto ask2 = std::make_unique<FixedPolicy>(Decision::Kind::Ask, "ask2");
    PolicyChain chain;
    REQUIRE(chain.add(std::make_unique<FixedPolicy>(Decision::Kind::Allow, "ok")));
    REQUIRE(chain.add(std::move(ask1)));
    REQUIRE(chain.add(std::move(ask2)));
    CommandRequest req;
    req.program = "git";
    Decision d = chain.evaluate(req);
    CHECK(d.kind == Decision::Kind::Ask);
    CHECK_EQ(d.policy, std::string("ask1"));

    auto deny = std::make_unique<FixedPolicy>(Decision::Kind::Deny, "no");
    auto after = std::make_unique<FixedPolicy>(Decision::Kind::Allow, "after");
    FixedPolicy* after_ptr = after.get();   // still valid: the chain owns it now
    REQUIRE(chain.add(std::move(deny)));
    REQUIRE(chain.add(std::move(after)));
    d = chain.evaluate(req);
    CHECK(d.kind == Decision::Kind::Deny);
    CHECK_EQ(d.policy, std::string("no"));
    CHECK_EQ(after_ptr->calls, 0);     // never evaluated: the chain stopped at the Deny
}

TEST(pol_chain_nested){
    PolicyChain parent;
    auto child = std::make_unique<PolicyChain>();
    PolicyChain* rawChild = child.get();
    child->add(std::make_unique<AllowlistPolicy>(std::vector<std::string>{"Allow Me!","before"}));
    child->add(std::make_unique<ArgumentPatternPolicy>(std::vector<std::string>{"deny this"}));
    
    REQUIRE(parent.add(std::move(child)));

    parent.add(std::make_unique<AllowlistPolicy>(std::vector<std::string>{"after","Allow Me!"}));
    CommandRequest allowThisChild{"before",std::vector<std::string>{"Hello"},"id=0"};
    CommandRequest denyThisChild{"Allow Me!", std::vector<std::string>{"deny this"},"id=1"};
    CommandRequest allowThisParent{"Allow Me!",std::vector<std::string>{"a"},"id=2"};

    Decision d1 = parent.evaluate(allowThisChild);      //Will be allowed by child but not parent chain
    Decision d1_1 = rawChild->evaluate(allowThisChild);     //Double checks that it was allowed by child
    Decision d2 = parent.evaluate(denyThisChild);       //Denied by child
    Decision d3 = parent.evaluate(allowThisParent);     //Allowed all the way through

    CHECK_EQ(d1.kind, Decision::Kind::Deny);    //Should deny from the parent
    CHECK_EQ(d1_1.kind, Decision::Kind::Allow); //Should be allowed by the child
    
    CHECK_EQ(d2.kind, Decision::Kind::Deny);   //Should be denied by child
    
    CHECK_EQ(d3.kind, Decision::Kind::Allow);   //Allowed all the way through
}




// =========================================================================
// Skills — 2 examples
// =========================================================================

// Example 7 (graph): post-order load order, and a cycle message that lists
// only the nodes still on the DFS path (b is already finished).
TEST(graph_order_and_cycle_message) {
    DepGraph g;
    for (const char* n : {"a", "b", "c", "d"}) g.add_node(n);
    g.add_edge("a", "b");   // a requires b
    g.add_edge("a", "c");
    g.add_edge("b", "d");
    g.add_edge("c", "d");   // diamond: d is reached twice, but it is not a cycle
    std::vector<std::string> order;
    std::string error;
    REQUIRE(g.order_from("a", &order, &error));
    CHECK_EQ(order, (std::vector<std::string>{"d", "b", "c", "a"}));

    g.add_edge("c", "a");   // now a -> c -> a is a cycle
    CHECK(!g.order_from("a", &order, &error));
    CHECK_EQ(error, std::string("cycle: a -> c -> a"));
}

// Example 8 (index): rank by number of distinct query words; ties go to the
// smaller doc id.
TEST(index_ranking_and_ties) {
    SkillIndex ix;
    ix.add(0, "format numbers");
    ix.add(1, "csv report with table and numbers");
    ix.add(2, "table format");
    ix.add(3, "unrelated");
    CHECK_EQ(ix.search("csv table numbers", 10), (std::vector<std::size_t>{1, 0, 2}));
    CHECK_EQ(ix.search("format table", 1), (std::vector<std::size_t>{2}));
    REQUIRE(ix.postings("numbers") != nullptr);
    CHECK_EQ(*ix.postings("numbers"), (std::vector<std::size_t>{0, 1}));
}

// =========================================================================
// Your tests (at least 12 more). Spec §8 lists what they must cover.
// =========================================================================

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
