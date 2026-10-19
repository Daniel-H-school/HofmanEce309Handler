// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {
    
    std::vector<double> values; //Vector of values
    std::vector<CalcToken> ops; //Vector of operations


    for (int i = 0; i < tokens.size(); i++){    //Iterates through the tokens vector
        CalcToken token = tokens[i];    //Gets a token 
        if (token.kind == CalcToken::Kind::Number){ //If this token is a number
            values.push_back(token.value);  //Push onto values stack
        } else if (token.kind == CalcToken::Kind::LParen) { //If this is a LParen
            ops.push_back(token);   //Push onto operator stack
        } else if (token.kind == CalcToken::Kind::RParen) {
            CalcToken op = ops.back();  //Look at top of ops stack
            while (op.kind != CalcToken::Kind::LParen){ //Go down ops stack until the ( is reached)
                ops.pop_back();
//PICKUP HERE                double right = pop
            }
        }
    }


    *error = "calc_evaluate not implemented yet";   // TODO
    return false;
}
