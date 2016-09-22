#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace std;
typedef map<long long int, long long int, std::greater<long long int> > QuantityMap;
enum OperationType {BUY, 
                    SELL, 
                    CANCEL, 
                    MODIFY, 
                    PRINT, 
                    INVALID_OPERATION
                   };

enum OrderType {GFD, 
                IOC, 
                INVALID
               };

/************************************************************************
name    : getType
purpose : converts a string to OperationType enum.
@inputs : str -> string whose OperationType needs to be fetched.
@return : OperationType enum
************************************************************************/
OperationType getType(const string& str) {
    if(str == "BUY") return BUY;
    else if(str == "SELL") return SELL;
    else if(str == "CANCEL") return CANCEL;
    else if(str == "MODIFY") return MODIFY;
    else if(str == "PRINT") return PRINT;
    else return INVALID_OPERATION;
}

/************************************************************************
name    : getOrderType
purpose : converts a string to OrderType enum.
@inputs : str -> string whose OrderType needs to be fetched.
@return : OrderType enum
************************************************************************/
OrderType getOrderType(const string& str) {
    if(str == "GFD") return GFD;
    else if(str == "IOC") return IOC;
    else return INVALID;
}

/************************************************************************
name    : tokenizeString
purpose : converts an input string to tokens, separated by a delimeter.
@inputs : str -> string that needs to be tokenized.
          inputParams -> vector that needs to be populated with the tokens.
          delimeter -> character or a set of characters at which the input
                       string needs to be tokenized.
@return : None.
************************************************************************/
void tokenizeString(const string& str, 
                    vector<string>& inputParams, 
                    const string& delimeter) {
    inputParams.clear();
    size_t pos = str.find(delimeter);
    if(pos == string::npos) {
        inputParams.push_back(str);
        return;
    }

    size_t initial = pos;
    inputParams.push_back(str.substr(0, pos));
    while(pos != string::npos) {
        pos = str.find(delimeter, initial+1);
        if(pos == string::npos) {
            inputParams.push_back(str.substr(initial+1, str.size()-initial-1));
            break;
        }
        else {
            inputParams.push_back(str.substr(initial+1, pos-initial-1));
            initial = pos;
        }
    }
}

/************************************************************************
name    : Class Engine, Class Order (nested)
purpose : Singleton class to create an engine object and use it for all
          processing. 
          - Engine does all the higher level processing.
          - No more than 1 instance of Engine is allowed.
          - The 5 operations defined in OperationType are executed by
            engine.
          - Object of Class Order is created everytime a SELL / BUY 
            operation is encountered.
************************************************************************/
class Engine {
    class Order {
        OrderType orderType_;
        long long int price_;
        long long int quantity_;
        string orderID_;

        public:
        // constructor
        Order(): orderType_(INVALID), price_(0), quantity_(0), orderID_("") {}
 
        // constructor
        Order(const vector<string>& inputParams) {
            orderType_ = ::getOrderType(inputParams[1]);
            price_ = stoll(inputParams[2]); 
            quantity_ = stoll(inputParams[3]);
            orderID_ = inputParams[4];
        }

        // getters
        inline long long int getPrice() const { return price_; }
        inline long long int getQuantity() const { return quantity_; }
        inline string getOrderID() const { return orderID_; }
        inline OrderType getOrderType() const { return orderType_; }

        // setters
        inline void setPrice(long long int price) { price_ = price; }
        inline void setQuantity(long long int quantity) { quantity_ = quantity; }
        inline void setOrderID(string orderID) { orderID_ = orderID; }
        inline void setOrderType(OrderType orderType) { orderType_ = orderType; }
    }; // end order class
    
    // class to override the comparator of std::find operation used in MODIFY
    class FindByOrderID {
        string orderID;
        public:
        FindByOrderID(const string& str) : orderID(str) {}
        bool operator()(const Order* order) const {
            return order->getOrderID() == orderID;
        }
    };

    static Engine* _instance;
    
    list<Order*> buyOrders;
    list<Order*> sellOrders;

    Engine() {}
    Engine(const Engine& e) {}
    void operator=(const Engine& e);

    public:
        
    // Engine is a singleton class
    static Engine* getInstance() {
        if(!_instance) _instance = new Engine();
        return _instance;
    }
    inline list<Order*>& getSellOrders() { return sellOrders; }
    inline list<Order*>& getBuyOrders() { return buyOrders; }

    bool validateInputs(const vector<string>&);
    void createAndTradeOrder(const vector<string>&);
    void modifyOrCancelOrder(const vector<string>&, bool = false);
    void printOrderBook();
    void printOrders(const OperationType&);
}; // end engine class

Engine* Engine::_instance = NULL;

/************************************************************************
name    : createAndTradeOrder
purpose : This method is invokved whenever BUY | SELL operation is
          encountered. It prints the following as soon as an order is
          seen which can be traded with the previous orders in the order
          book.
          TRADE orderID1 quantity1 price1 orderID2 quantity2 price2
          where, orderID1 is seen before orderID2.
@inputs : inputParams -> tokenized input received from stdin
@return : None.
************************************************************************/
void Engine::createAndTradeOrder(const vector<string>& inputParams) {
    if(inputParams.size() != 5) 
        return;
    
    Order* newOrder = new Order(inputParams);   
    if(!newOrder || 
       newOrder->getQuantity() <= 0 || 
       newOrder->getPrice() <= 0 || 
       newOrder->getOrderType() == INVALID) {
        return;
    }
    
    list<Order*> *toProcess = NULL, *toAdd = NULL;
    
    OperationType type = getType(inputParams[0]);
    if(type == BUY) {
        toProcess = &sellOrders;
        toAdd = &buyOrders;
    }
    else if(type == SELL) {
        toProcess = &buyOrders;
        toAdd = &sellOrders;
    }
    else {
        return;
    }
    
    for(list<Order*>::iterator pIter = toProcess->begin(); 
        pIter != toProcess->end(); 
        ++pIter) {
        
        Order* pOrder = *pIter; 
        if(!pOrder) {
            continue;
        }        
        
        if((type == BUY && newOrder->getPrice() < pOrder->getPrice()) || 
           (type == SELL && pOrder->getPrice() < newOrder->getPrice())) {
            continue;
        }
        
        long long int quantity = min(newOrder->getQuantity(), 
                                     pOrder->getQuantity());
        string result = "TRADE"; 
        string r1 = pOrder->getOrderID() + 
            " " + to_string(pOrder->getPrice()) + 
            " " + to_string(quantity);
        
        string r2 = newOrder->getOrderID() + 
            " " + to_string(newOrder->getPrice()) + 
            " " + to_string(quantity);

        // priority of newOrder will always be less than pOrders
        result += " " + r1 + " " + r2;
        cout<<result<<endl;

        pOrder->setQuantity(pOrder->getQuantity()-quantity);
        if(pOrder->getQuantity() == 0) {
            pIter = toProcess->erase(pIter);
        }

        newOrder->setQuantity(newOrder->getQuantity()-quantity);
        if(newOrder->getQuantity() == 0) {
            break;
        }
    }

    if(newOrder && 
       newOrder->getOrderType() == GFD && 
       newOrder->getQuantity() > 0 && 
       newOrder->getPrice() > 0) {
        toAdd->push_back(newOrder);
    }
}

/************************************************************************
name    : modifyOrCancelOrder
purpose : invoked whenever MODIFY / CANCEL operation is encountered.
          This method either updated the order or deletes it from the
          buyOrders / sellOrders lists defined in Engine class.
@inputs : inputParams -> tokenized input received from stdin
          cancelOrder -> boolean variable
                         false : default | OperationType = MODIFY
                         true : OperationType = CANCEL
@return : None.
************************************************************************/
void Engine::modifyOrCancelOrder(const vector<string>& inputParams, 
                                 bool cancelOrder) {
    if(inputParams.size() != 2 && inputParams.size() != 5) 
        return;
        
    list<Order*>::iterator it = std::find_if(buyOrders.begin(), 
                                             buyOrders.end(), 
                                             FindByOrderID(inputParams[1])); 

    list<Order*>* temp = NULL;
    if(it != buyOrders.end()) {
        temp = &buyOrders;
    }
    else {
        it = std::find_if(sellOrders.begin(), 
                          sellOrders.end(), 
                          FindByOrderID(inputParams[1]));    
        if(it != sellOrders.end()) {
            temp = &sellOrders;
        }
    }
    
    if(!temp)
        return;
 
    if(cancelOrder) {
        temp->erase(it);
    }
    else {
        // modify the order
        (*it)->setPrice(stoll(inputParams[3]));
        (*it)->setQuantity(stoll(inputParams[4]));
        
        // if modified order has both quantity and price > 0,
        // push into relevant container.
        if((*it)->getPrice() > 0 && (*it)->getQuantity() > 0) {
            OperationType modifyOrderType = getType(inputParams[2]);
            if(modifyOrderType == BUY) { buyOrders.push_back(*it); }
            else if(modifyOrderType == SELL) { sellOrders.push_back(*it); }
        }
        
        temp->erase(it);
    }
}

/************************************************************************
name    : printOrderBook
purpose : wrapper function for printOrders
          prints the order book in the following format:
          SELL:
          price1 quantity1
          price2 quantity2
          BUY:
          price3 quantity3
          price4 quantity4
          where, price1 > price2 and price3 > price4 (descending order)
@inputs : None.
@return : None.
************************************************************************/
void Engine::printOrderBook() {
    cout<<"SELL:"<<endl;
    printOrders(SELL);
    cout<<"BUY:"<<endl;
    printOrders(BUY);
}

/************************************************************************
name    : printOrders
purpose : prints the order book in the following format:
          SELL:
          price1 quantity1
          price2 quantity2
          BUY:
          price3 quantity3
          price4 quantity4
          where, price1 > price2 and price3 > price4 (descending order)
@inputs : OperationType - > BUY | SELL
@return : None.
************************************************************************/
void Engine::printOrders(const OperationType& type) {
    list<Order*>* temp = NULL;
    if(type == SELL) 
        temp = &getSellOrders();
    else if(type == BUY)
        temp = &getBuyOrders();
       
    if(!temp) return;
       
    QuantityMap priceQuantityMap;
    for(list<Order*>::iterator it = temp->begin(); it != temp->end(); ++it) {
        Order* top = *it;
        if(!top) 
            continue;
        priceQuantityMap[top->getPrice()] += top->getQuantity();
    }
    for(QuantityMap::iterator it = priceQuantityMap.begin(); 
        it != priceQuantityMap.end(); 
        ++it) {
        cout<<it->first<<" "<<it->second<<endl;
    }
}

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