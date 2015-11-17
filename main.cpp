#include <iostream>
#include <vector>
#include "function.h"
using namespace std;

int main()
{
// string s="2+sin(cos(x+3)+0.5)+1";
// ExpressionEvaluator ee;
// ee.evaluate(s);
    Expression *theX = new VariableX;
    Expression *One = new Constant(1);
    Expression *Two = new Constant(1);
    vector<double> para;
    para.push_back(1);
    para.push_back(2);
    para.push_back(3);
    Expression *poly = new Polynome(para);
    Expression *fun = new Operator('+', One, theX);
    Expression *fun2=new Operator('*',poly,fun);
    cout<<fun2->stringPrint()<<endl;
    cout<<(*fun2)(1.5)<<endl;
    cout<<fun2->diff()->stringPrint()<<endl;
    return 0;
}
