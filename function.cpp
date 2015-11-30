#include "function.h"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <algorithm>
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
    assert(para.size() >= 2);
    if (para[0] != 0)
        ss<<para[0];
    // special treatment to the second term
    // not "ax^1", but "ax"
    if (para[1] != 0) {
        if (para[1] > 0)
            ss<<"+";
        ss<<para[1]<<"x";
    }

    for (int i = 2; i < para.size(); i++) {
        if (para[i] == 0) continue;
        if (para[i] > 0)
            ss<<"+";
        ss<<para[i]<<"x^"<<i;
    }

    string s("Poly[");
    s += ss.str();
    s += "]";
    output += s;
}

Polynome::Polynome(double a, double b) :
    Expression(TypePoly)
{
    if (a == 0)
        throw invalid_argument("Degenerate case not allowed, Polynome can't be constant");
    this->para.push_back(b);
    this->para.push_back(a);
}

Polynome::Polynome(const vector<double> &parametre) :
    Expression(TypePoly)
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
        else
            break;
    }
    assert(para.size() >= 2);
}

bool Polynome::CanonicalEqualToSameType(Expression *other)
{
    assert(other->nodeType() == TypePoly);
    Polynome *p = static_cast<Polynome *>(other);
    if (para.size() != p->para.size()) {
        return false;
    } else {
        for (int i = para.size()-1; i >= 0; i--) {
            if (para[i] != p->para[i])
                return false;
        }
        return true;
    }
}

bool Polynome::CanonicalSmallerThanSameType(Expression *other)
{
    assert(other->nodeType() == TypePoly);
    Polynome *p = static_cast<Polynome *>(other);
    if (para.size() < p->para.size()) {
        return true;
    } else {
        for (int i = para.size()-1; i >= 0; i--) {
            if (para[i] < p->para[i])
                return true;
        }
        return false;
    }
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

bool Constant::CanonicalEqualToSameType(Expression *other)
{
    assert(other->nodeType() == TypeConstant);
    const Constant *p = static_cast<const Constant *>(other);
    return c == p->c;
}

bool Constant::CanonicalSmallerThanSameType(Expression *other)
{
    assert(other->nodeType() == TypeConstant);
    const Constant *p = static_cast<const Constant *>(other);
    return c < p->c;
}

void Constant::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    stringstream ss;
    ss<<this->c;
    output += ss.str();
}

bool Expression::CanonicalEqualTo(Expression *other)
{
    if (this->nodeType() != other->nodeType())
        return false;
    return this->CanonicalEqualToSameType(other);
}

bool Expression::CanonicalSmallerThan(Expression *other)
{
    if (this->nodeType() < other->nodeType())
        return true;
    else if (this->nodeType() > other->nodeType())
        return false;
    else
        return this->CanonicalSmallerThanSameType(other);
}

string Expression::stringPrint() const
{
    string s;
    recursivePrint(s, OperatorPrecedence::None);
    return s;
}

bool Trigo::CanonicalEqualToSameType(Expression *other)
{
    assert(other->nodeType() == TypeTrigo);
    Trigo *p = static_cast<Trigo *>(other);
    return this->trigoType == p->trigoType;
}

bool Trigo::CanonicalSmallerThanSameType(Expression *other)
{
    assert(other->nodeType() == TypeTrigo);
    Trigo *p = static_cast<Trigo *>(other);
    return this->trigoType < p->trigoType;
}

Expression *Trigo::diff() const
{
    switch (trigoType) {
    case Sin:
        return new Trigo(Cos);
        break;
    case Cos:
    {
        Expression *c = new Trigo(Sin);
        return new Multiplication(new Constant(-1), c);
        break;
    }
    case Tan:
    {
        // 1/(cos*cos)
        Expression *c1 = new Trigo(Cos);
        Expression *c2 = new Trigo(Cos);
        Expression *m = new Multiplication(c1, c2);
        return new Divide(new Constant(1), m);
        break;
    }
    default:
        assert(false);
    }
}

Expression *Trigo::clone() const
{
    return new Trigo(*this);
}

double Trigo::operator ()(double x) const
{
    switch (trigoType) {
    case Sin:
        return sin(x);
    case Cos:
        return cos(x);
    case Tan:
        return tan(x);
    default:
        assert(false);
    }
}

string Trigo::functionName() const
{
    switch (trigoType) {
    case Sin:
        return "sin";
    case Cos:
        return "cos";
    case Tan:
        return "tan";
    default:
        assert(false);
    }
}

Expression *Logarithm::diff() const
{
    // 1/x
    return new Divide(new Constant(1), new VariableX);
}

Expression *Logarithm::clone() const
{
    return new Logarithm;
}

double Logarithm::operator ()(double x) const
{
    return log(x);
}

string Logarithm::functionName() const
{
    return string("ln");
}

Expression *Exponential::diff() const
{
    return new Exponential;
}

Expression *Exponential::clone() const
{
    return new Exponential;
}

double Exponential::operator ()(double x) const
{
    return exp(x);
}

string Exponential::functionName() const
{
    return string("exp");
}

void ElementryFunction::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    output += functionName();
    output += "(x)";
}

void ElementryFunction::compositionPrint(string &output, const Expression *innerFunction) const
{
    string inner;
    innerFunction->recursivePrint(inner, OperatorPrecedence::None);
    output += functionName();
    output += "(";
    output += inner;
    output += ")";
}

double Addition::operator ()(double x) const
{
    double sum = 0;
    for (int i = 0; i < childrenList.size(); i++)
        sum += (*childrenList[i])(x);
    return sum;
}

void Addition::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    recursivePrintCommutative(output, order, OperatorPrecedence::AddSub, '+');
}

Expression *Addition::diff() const
{
    vector<Expression *> d;
    for (int i = 0; i < childrenList.size(); i++)
        d.push_back((*childrenList[i]).diff());
    return new Addition(d);
}

double Multiplication::operator ()(double x) const
{
    double prod = 1;
    for (int i = 0; i < childrenList.size(); i++)
        prod *= (*childrenList[i])(x);
    return prod;
}

void Multiplication::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    recursivePrintCommutative(output, order, OperatorPrecedence::MultiDivide, '*');
}

Expression *Multiplication::diff() const
{
    vector<Expression *> term;
    vector<Expression *> sum;
    for (int k = 0; k < childrenList.size(); k++) {
        for (int i = 0; i < childrenList.size(); i++) {
            if (i == k)
                term.push_back(childrenList[i]->diff());
            else
                term.push_back(childrenList[i]->clone());
        }
        sum.push_back(new Multiplication(term));
        term.clear();
    }
    return new Addition(sum);
}

void CommutativeOperators::recursivePrintCommutative(string &output,
                                                     OperatorPrecedence::Order order,
                                                     OperatorPrecedence::Order selfOrder,
                                                     char symbol) const
{
    bool closeParenthese = false;
    OperatorPrecedence::Order nextOrder;
    if (order >= selfOrder) {
        output.push_back('(');
        closeParenthese = true;
    }
    nextOrder = selfOrder;
    childrenList[0]->recursivePrint(output, nextOrder);
    for (int i = 1; i < childrenList.size(); i++) {
        output.push_back(symbol);
        childrenList[i]->recursivePrint(output, nextOrder);
    }
    if (closeParenthese)
        output.push_back(')');
}

void CommutativeOperators::construct(std::vector<Expression *> list)
{
    for (int i = 0; i < list.size(); i++) {
        if (list[i] == 0)
            throw invalid_argument("Null pointers in arguments");
    }
    this->childrenList = list;
}

void CommutativeOperators::construct(Expression *a, Expression *b)
{
    vector<Expression *> add;
    add.push_back(a);
    add.push_back(b);
    construct(add);
}

class SmallerFunctor
{
public:
    bool operator()(Expression *a, Expression *b)
    {
        return a->CanonicalSmallerThan(b);
    }
};
void CommutativeOperators::sortChildren()
{
    SmallerFunctor f;
    sort(childrenList.begin(), childrenList.end(), f);
}

bool CommutativeOperators::CanonicalEqualToSameType(Expression *other)
{
    assert(other->nodeType() == TypeAdd || other->nodeType() == TypeMulti);
    CommutativeOperators *p = static_cast<CommutativeOperators *>(other);
    int n = childrenList.size();
    int m = p->childrenList.size();
    if (n != m) {
        return false;
    } else {
        this->sortChildren();
        p->sortChildren();
        for (int i = 0; i < n; i++) {
            if (!childrenList[i]->CanonicalEqualTo(p->childrenList[i]))
                return false;
        }
        return true;
    }
}

bool CommutativeOperators::CanonicalSmallerThanSameType(Expression *other)
{
    assert(other->nodeType() == TypeAdd || other->nodeType() == TypeMulti);
    CommutativeOperators *p = static_cast<CommutativeOperators *>(other);
    int n = childrenList.size();
    int m = p->childrenList.size();
    if (n < m) {
        return true;
    } else if (n > m) {
        return false;
    } else {
        SmallerFunctor f;
        sort(childrenList.begin(), childrenList.end(), f);
        sort(p->childrenList.begin(), p->childrenList.end(), f);
        for (int i = 0; i < n; i++) {
            if (childrenList[i]->CanonicalSmallerThan(p->childrenList[i]))
                return true;
        }
        return false;
    }
}

Expression *CommutativeOperators::clone() const
{
    vector<Expression *> cl;
    for (int i = 0; i < childrenList.size(); i++)
        cl.push_back((*childrenList[i]).clone());
    return new Addition(cl);
}

CommutativeOperators::~CommutativeOperators()
{
    for (int i = 0; i < childrenList.size(); i++)
        delete childrenList[i];
}

Divide::Divide(Expression *numerator, Expression *denominator) :
    Expression(TypeDivide)
{
    this->numerator = numerator;
    this->denominator = denominator;
    if (!numerator || !denominator)
        throw invalid_argument("null argument in Divide");
}

bool Divide::CanonicalEqualToSameType(Expression *other)
{
    assert(other->nodeType() == TypeDivide);
    Divide *p = static_cast<Divide *>(other);
    return this->denominator->CanonicalEqualTo(p->denominator)
           && this->numerator->CanonicalEqualTo(p->numerator);
}

bool Divide::CanonicalSmallerThanSameType(Expression *other)
{
    assert(other->nodeType() == TypeDivide);
    Divide *p = static_cast<Divide *>(other);
    if (this->denominator->CanonicalSmallerThan(p->denominator))
        return true;
    else if (this->numerator->CanonicalSmallerThan(p->denominator))
        return true;
    else
        return false;
}

double Divide::operator ()(double x) const
{
    return (*numerator)(x)/(*denominator)(x);
}

void Divide::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    bool closeParenthese = false;
    OperatorPrecedence::Order nextOrder;
    if (order >= OperatorPrecedence::MultiDivide) {
        output.push_back('(');
        closeParenthese = true;
    }
    nextOrder = OperatorPrecedence::MultiDivide;
    numerator->recursivePrint(output, nextOrder);
    output.push_back('/');
    denominator->recursivePrint(output, nextOrder);
    if (closeParenthese)
        output.push_back(')');
}

Expression *Divide::diff() const
{
    Expression *df_g = new Multiplication(numerator->diff(), denominator->clone());
    Expression *f_dg = new Multiplication(numerator->clone(), denominator->diff());
    Expression *g2 = new Multiplication(denominator->clone(), denominator->clone());
    Expression *dfg_fdg = new Addition(df_g, new Multiplication(new Constant(-1), f_dg));
    return new Divide(dfg_fdg, g2);
}

Expression *Divide::clone() const
{
    return new Divide(numerator->clone(), denominator->clone());
}

Divide::~Divide()
{
    delete numerator;
    delete denominator;
}

Composition::Composition(Expression *left, Expression *right) :
    Expression(TypeCompo), left(left), right(right)
{
    if (!left || !right)
        throw invalid_argument("null pointer in Composition");
}

bool Composition::CanonicalEqualToSameType(Expression *other)
{
    assert(other->nodeType() == TypeCompo);
    Composition *p = static_cast<Composition *>(other);
    return this->left->CanonicalEqualTo(p->left)
           && this->right->CanonicalEqualTo(p->right);
}

bool Composition::CanonicalSmallerThanSameType(Expression *other)
{
    assert(other->nodeType() == TypeCompo);
    Composition *p = static_cast<Composition *>(other);
    if (this->left->CanonicalSmallerThan(p->left))
        return true;
    else if (this->right->CanonicalSmallerThan(p->right))
        return true;
    else
        return false;
}

void Composition::recursivePrint(string &output, OperatorPrecedence::Order order) const
{
    bool closeParenthese = false;
    OperatorPrecedence::Order nextOrder;
    if (order >= OperatorPrecedence::Composition) {
        output.push_back('(');
        closeParenthese = true;
    }
    nextOrder = OperatorPrecedence::Composition;
    ElementryFunction *f = dynamic_cast<ElementryFunction *>(left);
    if (f) {
        f->compositionPrint(output, right);
    } else {
        left->recursivePrint(output, nextOrder);
        output.push_back('o');
        right->recursivePrint(output, nextOrder);
    }
    if (closeParenthese)
        output.push_back(')');
}

Expression *Composition::diff() const
{
    Expression *dfog = new Composition(left->diff(), right->clone());
    Expression *dg = right->diff();
    return new Multiplication(dfog, dg);
}

Expression *Composition::clone() const
{
    return new Composition(left->clone(), right->clone());
}
