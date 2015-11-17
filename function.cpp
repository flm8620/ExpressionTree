#include "function.h"
#include <stdexcept>
#include <sstream>
#include <iostream>
using namespace std;
void ExpressionEvaluator::stringDecomposition()
{
    stringstream ss;
    ss<<this->expr;
    char c;
    while ((c = ss.peek()) != EOF) {
        if (c == ' ') {
            ss.get();
            continue;
        } else if (c >= '0' && c <= '9' || c == '.') {
            double x;
            ss>>x;
            if (ss.fail()) throw invalid_argument("invalid expression");
            symbolQueue.push(Symbol(x));
        } else if (c == '+') {
            ss.get();
            symbolQueue.push(Symbol(Symbol::Add));
        } else if (c == '-') {
            ss.get();
            symbolQueue.push(Symbol(Symbol::Sub));
        } else if (c == '*') {
            ss.get();
            symbolQueue.push(Symbol(Symbol::Multi));
        } else if (c == '/') {
            ss.get();
            symbolQueue.push(Symbol(Symbol::Divide));
        } else if (c == '(') {
            ss.get();
            symbolQueue.push(Symbol(Symbol::LeftParenthese));
        } else if (c == ')') {
            ss.get();
            symbolQueue.push(Symbol(Symbol::RightParenthese));
        } else if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
            // identifiers
            string name;
            char next;
            do {
                name.push_back(ss.get());
                next = ss.peek();
            } while (next >= 'a' && next <= 'z'
                     || next >= 'A' && next <= 'Z'
                     || next >= '0' && next <= '9'
                     || next == '_');
            if (name == "sin") symbolQueue.push(Symbol(Sin));
            else if (name == "cos") symbolQueue.push(Symbol(Cos));
            else if (name == "tan") symbolQueue.push(Symbol(Tan));
            else if (name == "exp") symbolQueue.push(Symbol(Exp));
            else if (name == "log") symbolQueue.push(Symbol(Log));
            else if (name == "x") symbolQueue.push(Symbol(Symbol::theVariableX));
            else throw invalid_argument("invalid function name");
        } else {
            throw invalid_argument("invalid symbol");
        }
    }
}

void ExpressionEvaluator::reversePolishNotation()
{
    // Shunting-yard algorithm
    stack<Symbol> operators;
    while (!symbolQueue.empty()) {
        Symbol sym = symbolQueue.front();
        symbolQueue.pop();
        switch (sym.type) {
        case Symbol::FunName:
            if (symbolQueue.front().type != Symbol::LeftParenthese)
                throw invalid_argument("funtion name must be followed by a '('");
            symbolPolish.push(sym);
            operators.push(Symbol(Symbol::ApplyFunction));
            break;
        case Symbol::Number:
        case Symbol::theVariableX:
            symbolPolish.push(sym);
            break;
        case Symbol::Add:
        case Symbol::Sub:
            while (!operators.empty() && operators.top().type != Symbol::LeftParenthese) {
                symbolPolish.push(operators.top());
                operators.pop();
            }
            operators.push(sym);
            break;
        case Symbol::Multi:
        case Symbol::Divide:
            while (!operators.empty() && operators.top().type != Symbol::LeftParenthese) {
                Symbol top = operators.top();
                if (top.type == Symbol::Add || top.type == Symbol::Sub)
                    break;
                symbolPolish.push(top);
                operators.pop();
            }
            operators.push(sym);
            break;
        case Symbol::RightParenthese:
            while (!operators.empty() && operators.top().type != Symbol::LeftParenthese) {
                symbolPolish.push(operators.top());
                operators.pop();
            }
            if (operators.empty())
                throw invalid_argument("a '(' is missed in the expression");
            assert(operators.top().type == Symbol::LeftParenthese);
            operators.pop();
            break;
        case Symbol::LeftParenthese:
            operators.push(sym);
            break;
        default:
            assert(false);
        }
    }// while
    while (!operators.empty()) {
        if (operators.top().type == Symbol::LeftParenthese)
            throw invalid_argument("a ')' is missed in the expression");
        assert(operators.top().type != Symbol::RightParenthese);
        symbolPolish.push(operators.top());
        operators.pop();
    }
}

Expression *ExpressionEvaluator::constructTree()
{
}

Expression *ExpressionEvaluator::evaluate(const std::string &s)
{
    pos = 0;
    while (!symbolPolish.empty())
        symbolPolish.pop();
    while (!symbolQueue.empty())
        symbolQueue.pop();
    if (s.size() == 0)
        throw invalid_argument("empty string");
    expr = s;
    stringDecomposition();
    reversePolishNotation();
    return constructTree();
}

void Polynome::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    stringstream ss;
    // special treatment to the first constant
    // not "ax^0", but "a"
    vector<double>::const_iterator it = para.begin();
    ss<<*it;
    it++;
    // special treatment to the second term
    // not "ax^1", but "ax"
    if (para.size() >= 2) {
        ss<<"+"<<*it<<"x";
        it++;
        int power = 2;
        for (; it != para.end(); it++) {
            ss<<"+";
            ss<<*it<<"x^"<<power;
            power++;
        }
    }
    string s("Poly[");
    s += ss.str();
    s += "]";
    output+=s;
}

Polynome::Polynome(double a, double b)
{
    if (a == 0)
        throw invalid_argument("Degenerate case not allowed, Polynome can't be constant");
    this->para.push_back(b);
    this->para.push_back(a);
}

Polynome::Polynome(const vector<double> &parametre)
{
    this->para = parametre;
    bool isConst = true;
    if (para.size() <= 1) {
        isConst = true;
    } else {
        for (int i = 1; i < para.size(); i++)
            if (para[i] != 0)
                isConst = false;
    }
    if (isConst)
        throw invalid_argument("Degenerate case not allowed, Polynome can't be constant");
    // clear zeros at the end of list
    for (int i = para.size()-1; i >= 0; i--) {
        if (para[i] == 0)
            para.pop_back();
    }
    assert(para.size() >= 2);
}

double Polynome::operator ()(double x) const
{
    assert(para.size() > 0);
    double xPowerK = 1;
    double sum = 0;
    for (vector<double>::const_iterator it = para.begin(); it != para.end(); it++) {
        sum += (*it)*xPowerK;
        xPowerK *= x;
    }
    return sum;
}

Expression *Polynome::diff() const
{
    assert(para.size() >= 2);
    vector<double> temp;
    vector<double>::const_iterator it = para.begin();
    it++;
    int k = 1;
    for (; it != para.end(); it++) {
        temp.push_back(k*(*it));// d(a*x^k)=a*k*x^(k-1)
        k++;
    }
    assert(temp.size() >= 0);// polynome isn't allowed to be constant
    if (temp.size() == 1)
        return new Constant(temp[0]);// function is affine = ax+b, df=a
    else
        return new Polynome(temp);
}

Expression *Polynome::clone() const
{
    return new Polynome(*this);
}

void Operator::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    bool closeParenthese = false;
    OperatorPrecedence::Order nextOrder;
    switch (type) {
    case Add:
    case Sub:
    case Minus:
        if (order > OperatorPrecedence::AddSub) {
            output.push_back('(');
            closeParenthese = true;
        }
        nextOrder = OperatorPrecedence::AddSub;
        break;
    case Multi:
    case Divide:
        if (order > OperatorPrecedence::MultiDivide) {
            output.push_back('(');
            closeParenthese = true;
        }
        nextOrder = OperatorPrecedence::MultiDivide;
        break;
    case Power:
        if (order > OperatorPrecedence::Power) {
            output.push_back('(');
            closeParenthese = true;
        }
        nextOrder = OperatorPrecedence::Power;
        break;
    default:
        assert(false);
    }
    if (leftChild)// e.g. -A, leftChild=NULL
        leftChild->recursivePrint(output, nextOrder);
    output.push_back(symbol);
    rightChild->recursivePrint(output, nextOrder);
    if(closeParenthese)
        output.push_back(')');
}

Operator::Operator(char op, Expression *left, Expression *right)
{
    if (!left || !right)
        throw invalid_argument("invalid expression");
    switch (op) {
    case '+':
        type = Add;
        opFunction = &opPlus;
        break;
    case '-':
        type = Sub;
        opFunction = &opSub;
        break;
    case '*':
        type = Multi;
        opFunction = &opMulti;
        break;
    case '/':
        type = Divide;
        opFunction = &opDivide;
        break;
    default:
        throw invalid_argument("invalid operator");
    }
    leftChild = left;
    rightChild = right;
    symbol = op;
}

Operator::Operator(char op, Expression *right)
{
    if (op != '-')
        throw invalid_argument("invalid unary operator");
    if (!right)
        throw invalid_argument("invalid expression");
    type = Minus;
    symbol = op;
    opFunction = opMinus;
    leftChild = NULL;
    rightChild = right;
}

double Operator::operator ()(double x) const
{
    return (this->*opFunction)(x);
}

Expression *Operator::diff() const
{
    switch (type) {
    case Add:
    case Sub:
        assert(leftChild && rightChild);
        return new Operator(this->symbol, leftChild->diff(), rightChild->diff());
        break;
    case Multi:
    {
        assert(leftChild && rightChild);
        Expression *df_g = new Operator('*', leftChild->diff(), rightChild->clone());
        Expression *f_dg = new Operator('*', leftChild->clone(), rightChild->diff());
        return new Operator('+', df_g, f_dg);
        break;
    }
    case Divide:
    {
        assert(leftChild && rightChild);
        Expression *df_g = new Operator('*', leftChild->diff(), rightChild->clone());
        Expression *f_dg = new Operator('*', leftChild->clone(), rightChild->diff());
        Expression *numerator = new Operator('+', df_g, f_dg);
        Expression *g_sq = new Operator('*', rightChild->clone(), rightChild->clone());
        return new Operator('/', numerator, g_sq);
        break;
    }
    case Minus:
        assert(!leftChild && rightChild);
        return new Operator(this->symbol, rightChild->diff());
        break;
    default:
        assert(false);
    }
}

Expression *Operator::clone() const
{
    Expression *newLeft = leftChild ? leftChild->clone() : NULL;
    Expression *newRight = rightChild ? rightChild->clone() : NULL;
    return new Operator(this->symbol, newLeft, newRight);
}

void Composition::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    leftChild->recursivePrint(output, OperatorPrecedence::None);
    output.push_back('(');
    rightChild->recursivePrint(output, OperatorPrecedence::None);
    output.push_back(')');
}

Composition::Composition(Expression *f, Expression *g)
{
    if (!f || !g)
        throw invalid_argument("invalid expression");
    leftChild = f;
    rightChild = g;
}

double Composition::operator ()(double x) const
{
    return (*leftChild)((*rightChild)(x));
}

Expression *Composition::diff() const
{
    Expression *df = leftChild->diff();
    Expression *dg = rightChild->diff();
    Expression *dfog = new Composition(df, rightChild->clone());
    return new Operator('*', dfog, rightChild->diff());
}

Expression *Composition::clone() const
{
    return new Composition(leftChild->clone(), rightChild->clone());
}

void Constant::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    stringstream ss;
    ss<<this->c;
    output+=ss.str();
}

string Expression::stringPrint() const
{
    string s;
    recursivePrint(s, OperatorPrecedence::None);
    return s;
}
