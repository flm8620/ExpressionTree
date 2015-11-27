#ifndef FUNCTION_H
#define FUNCTION_H
#include <stack>
#include <queue>
#include <string>
#include <cassert>
#include <vector>
#include <cmath>
enum FunctionName {
    Sin,
    Cos,
    Tan,
    Exp,
    Log,
};
struct OperatorPrecedence {
    enum Order {
        None = 0,
        AddSub = 1,
        MultiDivide = 2,
        Power = 3,
        Composition = 4
    };
};

struct Symbol {
    enum Type {
        Add,
        Sub,// -B or A-B
        Multi,
        Divide,
        LeftParenthese,
        RightParenthese,
        FunName,
        Number,
        theVariableX,
        ApplyFunction,
    } type;
    double number;
    FunctionName funName;
    Symbol(Type type) : type(type){assert(type != FunName && type != Number); }
    Symbol(double x) : number(x), type(Number){}
    Symbol(FunctionName name) : funName(name), type(FunName){}
};

class Expression
{
protected:
    Expression *leftChild;
    Expression *rightChild;

public:
    Expression(){leftChild=0;rightChild=0;}
    virtual Expression *diff() const = 0;
    virtual Expression *clone() const = 0;
    virtual double operator ()(double x) const = 0;
    virtual void recursivePrint(std::string &output, OperatorPrecedence::Order order) const = 0;
    virtual std::string stringPrint() const;
    virtual ~Expression(){delete leftChild; delete rightChild; }
    // virtual bool TryToSimplifyWith(Expression *opt) const = 0;
};

class Operator : public Expression
{
public:
    enum Type {
        Add,            // A+B
        Sub,            // A-B
        Multi,          // A*B
        Divide,         // A/B
        Power,          // A^B
        Minus,          // -A
        Composition     // f(g(x))
    };
private:
    Type type;
    double opPlus(double x) const {return (*leftChild)(x)+(*rightChild)(x); }
    double opSub(double x) const {return (*leftChild)(x)-(*rightChild)(x); }
    double opMulti(double x) const {return (*leftChild)(x)*(*rightChild)(x); }
    double opDivide(double x) const {return (*leftChild)(x)/(*rightChild)(x); }
    double opPower(double x) const {return pow((*leftChild)(x), (*rightChild)(x)); }
    double opMinus(double x) const {return -(*leftChild)(x); }
    double opComposition(double x) const {return (*leftChild)((*rightChild)(x)); }

    // pointer to member function
    double (Operator::*opFunction)(double) const;
    // if not in c++11
    void construct(Type optType, Expression *left, Expression *right);
public:

    Operator(Type optType, Expression *left, Expression *right);
    Operator(char op, Expression *left, Expression *right);
    Operator(char op, Expression *right);
    double operator ()(double x) const;
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    Expression *diff() const;
    Expression *clone() const;
};

class Constant : public Expression
{
    double c;
public:
    Constant(double c) : c(c){}
    double operator ()(double x) const {return c; }
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    Expression *diff() const {return new Constant(0); }
    Expression *clone() const {return new Constant(*this); }
};

class VariableX : public Expression
{
public:
    VariableX(){}
    double operator ()(double x) const {return x; }
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const
    {
        output.push_back('x');
    }

    Expression *diff() const {return new Constant(1); }
    Expression *clone() const {return new VariableX; }
};

class Polynome : public Expression
{
    std::vector<double> para;
protected:
    // only for Affine
    Polynome(double a, double b);
public:
    Polynome(const std::vector<double> &parametre);
    double operator ()(double x) const;
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    Expression *diff() const;
    Expression *clone() const;
    virtual ~Polynome()
    {
    }
};

class Affine : public Polynome
{
public:
    Affine(double a, double b) : Polynome(a, b){}
};

class ElementryFunction : public Expression
{
public:
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    // used in the case of composition, such as sin( cos(x) )
    void compositionPrint(std::string &output, const Expression *innerFunction) const;
    virtual std::string functionName() const = 0;
};
class Trigo : public ElementryFunction
{
public:
    enum Type {
        Sin, Cos, Tan
    };
private:
    Type type;
public:
    Trigo(Type trigoType) : type(trigoType){}
    virtual Expression *diff() const;
    virtual Expression *clone() const;
    virtual double operator()(double x) const;
    std::string functionName() const;
};
class Logarithm : public ElementryFunction
{
public:
    virtual Expression *diff() const;
    virtual Expression *clone() const;
    virtual double operator ()(double x) const;
    std::string functionName() const;
};
class Exponential : public ElementryFunction
{
public:
    virtual Expression *diff() const;
    virtual Expression *clone() const;
    virtual double operator ()(double x) const;
    std::string functionName() const;
};

class ExpressionEvaluator
{
private:
    std::string expr;
    int pos;
    std::queue<Symbol> symbolQueue;
    std::queue<Symbol> symbolPolish;
    void stringDecomposition();
    void reversePolishNotation();
    Expression *constructTree();
public:
    Expression *evaluate(const std::string &s);
};

class Function
{
public:
    Function();
};

#endif // FUNCTION_H
