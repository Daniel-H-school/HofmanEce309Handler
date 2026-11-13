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

// =========================================================================
// Policy — 2 examples
// =========================================================================

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
