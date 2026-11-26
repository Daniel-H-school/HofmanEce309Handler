// src/policy/policy_chain.cpp — YOU IMPLEMENT. Spec §5.3.

#include "policy/policy_chain.h"

#include <utility>

bool PolicyChain::add(std::unique_ptr<Policy> child) {
    // TODO
    // Clue: a unique_ptr cannot be copied. Check for nullptr first, then
    // children_.push_back(std::move(child)). Plain push_back(child) gives a
    // long error mentioning "use of deleted function unique_ptr(const unique_ptr&)".

    //Adds the pointer to the child policy to the chain.
    if (child == nullptr){  //does a quick check that the passes parameter is valid
        return false;   //Note that it has failed
    } else {
        children_.push_back(std::move(child));  //Transfer ownership and move over the child policy to the chain
        return true;
    }
}

std::size_t PolicyChain::size() const noexcept { return children_.size(); }

Decision PolicyChain::evaluate(const CommandRequest& req) {
    // TODO
    // Clues:
    //  * Handle the empty chain before the loop (Deny, policy "chain").
    //  * Loop with  for (const auto& child : children_)  (the & matters:
    //    unique_ptr can't be copied) and call child->evaluate(req).
    //  * Don't keep a "most restrictive so far" maximum: that returns the
    //    LAST Ask. Instead: return a Deny the moment you see one; remember
    //    only the FIRST Ask; after the loop return that Ask if there was one.
    //  * The enum is nested: write Decision::Kind::Deny, not Deny or
    //    Decision::Deny.

    if (children_.empty()){ //If no children
        return Decision::deny("No children to evaluate", "chain");  //Deny
    } else {
        Policy* firstAsk = nullptr;
        Decision askRequest;
        for (const auto& child : children_){        //For each child policy in chain
            Decision childDecision = child->evaluate(req);      //Evaluate the policy for this child
            if (childDecision.kind == Decision::Kind::Deny){    //If this child denies the request,
                return childDecision;                           // return the decision ver batum
            } else if (childDecision.kind == Decision::Kind::Ask && firstAsk == nullptr){  //Else if the child needed to ask, and this is the first child to do so
                firstAsk = child.get(); //Get the first ask in a raw pointer
                askRequest = childDecision; //Store the request
            } else {    //If this policy was allowed or the second+ ask,
                continue;   //Goes on to next child
            }
        }
        if (firstAsk != nullptr){   //If there was an Ask request
            return askRequest;      //return the ask request
        }
        //If no asks and no denies
        return Decision::allow("No denials or ask requests", "chain");
    }
}
