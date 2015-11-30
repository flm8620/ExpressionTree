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
public:
    enum NodeType {
        TypeConstant,
        TypeVariable,
        TypeAdd,
        TypeMulti,
        TypeDivide,
        TypePower,
        TypeCompo,
        TypePoly,
        TypeTrigo,
        TypeExp,
        TypeLog
    };
    NodeType type;
public:
    Expression(NodeType type) : type(type){}
    NodeType nodeType(){return type; }
    bool CanonicalEqualTo(Expression *other);
    bool CanonicalSmallerThan(Expression *other);
    virtual bool CanonicalEqualToSameType(Expression *other) = 0;
    virtual bool CanonicalSmallerThanSameType(Expression *other) = 0;
    virtual Expression *diff() const = 0;
    virtual Expression *clone() const = 0;
    virtual double operator ()(double x) const = 0;
    virtual void recursivePrint(std::string &output, OperatorPrecedence::Order order) const = 0;
    virtual std::string stringPrint() const;
    virtual ~Expression(){}
    // return NULL if it can't simplify
    virtual Expression *TryToSimplifyAddingWith(Expression *right) const = 0;
    virtual Expression *TryToSimplifyMultiplingWith(Expression *right) const = 0;
    virtual Expression *TryToSimplifyDividedBy(Expression *right) const = 0;
    virtual Expression *TryToSimplifyDividing(Expression *left) const = 0;
    virtual Expression *TryToSimplifyPoweredBy(Expression *right) const = 0;
    virtual Expression *TryToSimplifyPowering(Expression *left) const = 0;
    virtual Expression *TryToSimplifyComposedBy(Expression *right) const = 0;
    virtual Expression *TryToSimplifyComposing(Expression *left) const = 0;
    virtual void simplify()=0;
};
class CommutativeOperators : public Expression
{
protected:
    std::vector<Expression *> childrenList;
    void recursivePrintCommutative(std::string &output, OperatorPrecedence::Order order,
                                   OperatorPrecedence::Order selfOrder, char symbol) const;
    void construct(std::vector<Expression *> list);
    void construct(Expression *a, Expression *b);
    void sortChildren();
public:
    CommutativeOperators(NodeType type) : Expression(type){}
    bool CanonicalEqualToSameType(Expression *other);
    bool CanonicalSmallerThanSameType(Expression *other);
    Expression *clone() const;
    virtual ~CommutativeOperators();
};

class Addition : public CommutativeOperators
{
public:
    Addition(std::vector<Expression *> &exprs) : CommutativeOperators(TypeAdd){construct(exprs); }
    Addition(Expression *a, Expression *b) : CommutativeOperators(TypeAdd){construct(a, b); }
    double operator ()(double x) const;
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    Expression *diff() const;
};

class Multiplication : public CommutativeOperators
{
public:
    Multiplication(std::vector<Expression *> &exprs) : CommutativeOperators(TypeMulti)
    {
        construct(exprs);
    }

    Multiplication(Expression *a, Expression *b) : CommutativeOperators(TypeMulti)
    {
        construct(a, b);
    }

    double operator ()(double x) const;
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    Expression *diff() const;
};

class Divide : public Expression
{
    Expression *numerator, *denominator;
public:
    Divide(Expression *numerator, Expression *denominator);
    bool CanonicalEqualToSameType(Expression *other);
    bool CanonicalSmallerThanSameType(Expression *other);
    double operator ()(double x) const;
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    Expression *diff() const;
    Expression *clone() const;
    ~Divide();
};
class Composition : public Expression
{
    Expression *left, *right;
public:
    Composition(Expression *left, Expression *right);
    bool CanonicalEqualToSameType(Expression *other);
    bool CanonicalSmallerThanSameType(Expression *other);
    double operator ()(double x) const {return (*left)((*right)(x)); }
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    Expression *diff() const;
    Expression *clone() const;
    ~Composition(){delete left; delete right; }
};

class Constant : public Expression
{
    double c;
public:
    Constant(double c) : Expression(TypeConstant), c(c){}
    bool CanonicalEqualToSameType(Expression *other);
    bool CanonicalSmallerThanSameType(Expression *other);
    double operator ()(double x) const {return c; }
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    Expression *diff() const {return new Constant(0); }
    Expression *clone() const {return new Constant(*this); }
};

class VariableX : public Expression
{
public:
    VariableX() : Expression(TypeVariable){}
    bool CanonicalEqualToSameType(Expression *other){return true; }
    bool CanonicalSmallerThanSameType(Expression *other){return false; }
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
    bool CanonicalEqualToSameType(Expression *other);
    bool CanonicalSmallerThanSameType(Expression *other);
    double operator ()(double x) const;
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    Expression *diff() const;
    Expression *clone() const;
    virtual ~Polynome()
    {
    }
};

class ElementryFunction : public Expression
{
public:
    ElementryFunction(NodeType type) : Expression(type){}
    void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
    // used in the case of composition, such as sin( cos(x) )
    void compositionPrint(std::string &output, const Expression *innerFunction) const;
    virtual std::string functionName() const = 0;
};
class Trigo : public ElementryFunction
{
public:
    enum TrigoType {
        Sin, Cos, Tan
    };
private:
    TrigoType trigoType;
public:
    Trigo(TrigoType trigoType) : ElementryFunction(TypeTrigo), trigoType(trigoType){}
    bool CanonicalEqualToSameType(Expression *other);
    bool CanonicalSmallerThanSameType(Expression *other);
    virtual Expression *diff() const;
    virtual Expression *clone() const;
    virtual double operator()(double x) const;
    std::string functionName() const;
};
class Logarithm : public ElementryFunction
{
public:
    Logarithm() : ElementryFunction(TypeLog){}
    bool CanonicalEqualToSameType(Expression *other){return true; }
    bool CanonicalSmallerThanSameType(Expression *other){return false; }
    virtual Expression *diff() const;
    virtual Expression *clone() const;
    virtual double operator ()(double x) const;
    std::string functionName() const;
};
class Exponential : public ElementryFunction
{
public:
    Exponential() : ElementryFunction(TypeExp){}
    bool CanonicalEqualToSameType(Expression *other){return true; }
    bool CanonicalSmallerThanSameType(Expression *other){return false; }
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
