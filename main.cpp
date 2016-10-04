#include <map>
#include <set>
#include <list>
#include <string>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <iostream>
#include "engine.h"

using namespace std;

/************************************************************************
name    : Main routine.
purpose : Program call begins here.
@inputs : None.
@return : 0 -> successful execution
************************************************************************/
int main() {
    Engine* engine = Engine::getInstance();
    string input;
    vector<string> inputParams;

    while(!std::cin.eof()) {
        getline(cin, input);
        tokenizeString(input, inputParams, " ");        

        switch(getType(inputParams[0])) {
            case BUY:
            case SELL:
                engine->createAndTradeOrder(inputParams);
                break;
            case MODIFY:
                engine->modifyOrCancelOrder(inputParams); 
                break;
            case CANCEL:
                engine->modifyOrCancelOrder(inputParams, true);
                break;
            case PRINT:
                if(inputParams.size() != 1) 
                    continue;
                engine->printOrderBook();
                break;
            default:
                break;
        }
    }
    return 0;
}
