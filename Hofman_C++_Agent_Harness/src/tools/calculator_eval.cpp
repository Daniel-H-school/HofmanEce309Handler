// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include <iostream>

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {
    
    std::vector<double> values; //Vector of values
    std::vector<CalcToken> ops; //Vector of operations
    

    for (std::size_t i = 0; i < tokens.size(); i++){    //Iterates through the tokens vector
        CalcToken token = tokens[i];    //Gets a token 
        if (token.kind == CalcToken::Kind::Number){ //If this token is a number
            values.push_back(token.value);  //Push onto values stack
        } else if 
            //If multiply or divide,
        (token.kind == CalcToken::Kind::Times || token.kind == CalcToken::Kind::Divide){ 
            if (ops.size() == 0 || ops.back().kind == CalcToken::Kind::LParen) ops.push_back(token);  //If ops stack is empty or a left parem, pushes this onto it
            else {
                CalcToken topOp = ops.back();
                //If top of stack is >= operator, apply it
                if (topOp.kind == CalcToken::Kind::Times || topOp.kind == CalcToken::Kind::Divide){
                    double right = values.back();
                    values.pop_back();
                    double left = values.back();
                    values.pop_back();
                    double toPush;
                    if (topOp.kind == CalcToken::Kind::Times) {
                        toPush = left * right;
                    } else {
                        if (right == 0){    //Division by zero exception
                            *error = "division by zero";
                            return false;
                        }
                        toPush = left / right;
                    }
                    values.push_back(toPush);
                    ops.pop_back(); //Pop out the applied operator
                }
                // Push this operator
                ops.push_back(token);
            }
        } else if 
            //If add or subtract
        (token.kind == CalcToken::Kind::Plus || token.kind == CalcToken::Kind::Minus) {
            if (ops.size() == 0 || ops.back().kind == CalcToken::Kind::LParen) ops.push_back(token);  //If ops stack is empty, pushes this onto it
            else {
                //Since ops stack not empty nor has LParen on top, all operators are >= plus and minus, so always apply the thing on the stack
                CalcToken topOp = ops.back();   //Reads top operator
                double right = values.back();   //Takes in left and right operands
                values.pop_back();
                double left = values.back();
                values.pop_back();
                double toPush;                  //Variable for computation of the operation
                switch (topOp.kind){            //Switch depending on operator to perform
                case CalcToken::Kind::Times:
                    toPush = left * right;
                    break;
                case CalcToken::Kind::Divide:
                    if (right == 0){        //Divide by zero exception
                        *error = "division by zero";
                        return false;
                    }
                    toPush = left / right;
                    break;
                case CalcToken::Kind::Plus:
                    toPush = left + right;
                    break;
                case CalcToken::Kind::Minus:
                    toPush = left - right;
                    break;
                default:    //If currently a Lparen or Rparen (or Number) --> impossible
                    break;  //Break to avoid warning
                }
                values.push_back(toPush);   //Push computed value to stack
                ops.pop_back();             //Pop out applied operator
                ops.push_back(token);            //Push current operator
            }
        } else if (token.kind == CalcToken::Kind::LParen) { //If this is a LParen
            ops.push_back(token);   //Push onto operator stack
        } else if (token.kind == CalcToken::Kind::RParen) {
            if (ops.size() == 0) {  //Fringe case of Rparen on empty stack
                *error = "incorrectly formatted CalcToken";
                return false;
            }
            CalcToken topOp = ops.back();  //Look at top of ops stack.
            while (topOp.kind != CalcToken::Kind::LParen){ //Go down ops stack until the '(' is reached -> may not need to be a while loop with current operator cases
                double right = values.back();       //Get left and right operands
                values.pop_back();
                double left = values.back();
                values.pop_back();
                double toPush;                      //Holds value to pushs after operation
                switch (topOp.kind){            //Switch depending on operator to perform
                case CalcToken::Kind::Times:
                    toPush = left * right;
                    break;
                case CalcToken::Kind::Divide:
                    if (right == 0){    //Divide by zero exception
                        *error = "division by zero";
                        return false;
                    }
                    toPush = left / right;
                    break;
                case CalcToken::Kind::Plus:
                    toPush = left + right;
                    break;
                case CalcToken::Kind::Minus:
                    toPush = left - right;
                    break;
                default:       //For Lparen, Rparen, and number -->should be impossible, ignored if not
                    break;
                }
                values.push_back(toPush);   //Push computed value to stack
                ops.pop_back();             //Pop out operated value
                topOp = ops.back();         //Resets topOp to go again
            }
            ops.pop_back();     //Pops out Lparen
        }
    }
    //Once tokens "end" and can back out of any remaining operators
    if (ops.size() == 0) { 
       *out = values.back();
       return true; 
    }
    while (!ops.empty()){ //Go down ops stack until the '(' is reached -> may not need to be a while loop with current operator cases
        CalcToken topOp = ops.back();  //Look at top of ops stack.    
        double right = values.back();       //Get left and right operands
        values.pop_back();
        double left = values.back();
        values.pop_back();
        double toPush;                      //Holds value to pushs after operation
        switch (topOp.kind){            //Switch depending on operator to perform
        case CalcToken::Kind::Times:
            toPush = left * right;
            break;
        case CalcToken::Kind::Divide:
            if (right == 0){    //Divide by zero exception
                *error = "division by zero";
                return false;
            }
            toPush = left / right;
            break;
        case CalcToken::Kind::Plus:
            toPush = left + right;
            break;
        case CalcToken::Kind::Minus:
            toPush = left - right;
            break;
        default:        //Shouldn't be a Lparen or Rparen or Number left in operator stack
            *error = "improperly formatted operation";  //Error in case
            return false;
        }
        values.push_back(toPush);   //Push computed value to stack
        ops.pop_back();             //Pop out operated value
    }
    //After the two stack method has completed,
    *out = values.back();   //Sets output value
    values.pop_back();      //Pops out value for easier vector deletion


    return true;
}
