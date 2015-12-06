#include <iostream>
#include "function.h"
#include "ExpressionEvaluator.h"
using namespace std;

int main()
{
    ExpressionEvaluator evaluator;
    Expression *e1;


    e1 = evaluator.evaluate("-1");


    delete e1;
    return 0;
}
