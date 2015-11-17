#ifndef FUNCTION_H
#define FUNCTION_H
#include <stack>
#include <queue>
#include <string>
#include <cassert>
#include <vector>
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
        Power = 3
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
    virtual Expression *diff() const = 0;
    virtual Expression *clone() const = 0;
    virtual double operator ()(double x) const = 0;
    virtual void recursivePrint(std::string &output, OperatorPrecedence::Order order) const = 0;
    virtual std::string stringPrint() const;
    virtual ~Expression(){delete leftChild; delete rightChild; }
};

class Operator : public Expression
{
    enum Type {
        Add,            // A+B
        Sub,            // A-B
        Multi,          // A*B
        Divide,         // A/B
        Power,          // A^B
        Minus,          // -A
    } type;
    char symbol;
    double opPlus(double x) const {return (*leftChild)(x)+(*rightChild)(x); }
    double opSub(double x) const {return (*leftChild)(x)-(*rightChild)(x); }
    double opMulti(double x) const {return (*leftChild)(x)*(*rightChild)(x); }
    double opDivide(double x) const {return (*leftChild)(x)/(*rightChild)(x); }
    double opMinus(double x) const {return -(*leftChild)(x); }
    // pointer to member function
    double (Operator::*opFunction)(double) const;

public:
    Operator(char op, Expression *left, Expression *right);
    Operator(char op, Expression *right);
    double operator ()(double x) const;
    void recursivePrint(std::string &output, OperatorPrecedence::Order order)const;
    Expression *diff() const;
    Expression *clone() const;
};

class Composition : public Expression
{
public:
    Composition(Expression *f, Expression *g);
    double operator ()(double x) const;
    void recursivePrint(std::string &output, OperatorPrecedence::Order order)const;
    Expression *diff() const;
    Expression *clone() const;
};

class Constant : public Expression
{
    double c;
public:
    Constant(double c) : c(c){}
    double operator ()(double x) const {return c; }
    void recursivePrint(std::string &output, OperatorPrecedence::Order order)const;
    Expression *diff() const {return new Constant(0); }
    Expression *clone() const {return new Constant(*this); }
};

class VariableX : public Expression
{
public:
    VariableX(){}
    double operator ()(double x) const {return x; }
    void recursivePrint(std::string &output, OperatorPrecedence::Order order)const
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
    void recursivePrint(std::string &output, OperatorPrecedence::Order order)const;
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
