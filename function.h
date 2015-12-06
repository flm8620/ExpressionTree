#ifndef FUNCTION_H
#define FUNCTION_H

#include <stack>
#include <queue>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <cmath>

struct OperatorPrecedence {
  enum Order {
    None = 0,
    AddSub = 1,
    MultiDivide = 2,
    Power = 3,
    PositiveNegative = 4,
    Composition = 5
  };
};

class Expression;

struct ExpressionComparator {
  bool operator()(Expression *left, Expression *right);
};

class Expression {
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
  Expression(NodeType type) : type(type) { }

  NodeType nodeType() { return type; }

  bool CanonicalEqualTo(Expression *other);
  bool CanonicalSmallerThan(Expression *other);
  virtual bool CanonicalEqualToSameType(Expression *other) = 0;
  virtual bool CanonicalSmallerThanSameType(Expression *other) = 0;
  virtual Expression *diff() const = 0;
  virtual Expression *clone() const = 0;
  virtual double operator()(double x) const = 0;
  virtual void recursivePrint(std::string &output, OperatorPrecedence::Order order) const = 0;
  virtual std::string stringPrint() const;

  virtual ~Expression() { }

  // return NULL if it can't simplify or the simplification doesn't
  // need change class type
  // Basic rules:
  // a*b+c*b = (a+c)*b
  // a*b+ b =(a+1)*b
  // b+b = 2*b
  // (a/b)*(c/d)=(a*c)/(b*d)
  // (a/b)*c = a*c / b
  // a/(c/d)=a*d/c
  // a/c/d=a/(c*d)
  // 0*a=0
  // 0+a=a
  // 1*a=1
  //
  virtual Expression *TrySimplifyAdding(Expression *right) = 0;
  virtual Expression *TrySimplifyMultiplying(Expression *right) = 0;
  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed) = 0;
};

typedef std::multiset<Expression *, ExpressionComparator> ExpressionSet;

class CommutativeOperators: public Expression {
protected:
  ExpressionSet childrenSet;
  void recursivePrintCommutative
      (std::string &output, OperatorPrecedence::Order order, OperatorPrecedence::Order selfOrder, char symbol) const;
  void construct(ExpressionSet children);
  void construct(Expression *a, Expression *b);
  //return true if children changed
  bool simplifyChildren();
public:
  CommutativeOperators(NodeType type) : Expression(type) {
  }

  bool CanonicalEqualToSameType(Expression *other);
  bool CanonicalSmallerThanSameType(Expression *other);
  Expression *clone() const;
  virtual ~CommutativeOperators();
};

class Addition: public CommutativeOperators {
public:
  Addition(ExpressionSet &exprs) : CommutativeOperators(TypeAdd) {
    construct(exprs);
  }

  Addition(Expression *a, Expression *b) : CommutativeOperators(TypeAdd) {
    construct(a, b);
  }

  double operator()(double x) const;
  void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
  Expression *diff() const;
  virtual Expression *TrySimplifyAdding(Expression *right);
  virtual Expression *TrySimplifyMultiplying(Expression *right);
  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed);
};

class Multiplication: public CommutativeOperators {
public:
  Multiplication(ExpressionSet &exprs) : CommutativeOperators(TypeMulti) {
    construct(exprs);
  }

  Multiplication(Expression *a, Expression *b) : CommutativeOperators(TypeMulti) {
    construct(a, b);
  }

  double operator()(double x) const;
  void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
  Expression *diff() const;
  virtual Expression *TrySimplifyAdding(Expression *right);
  virtual Expression *TrySimplifyMultiplying(Expression *right);
  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed);
};

class Division: public Expression {
  Expression *numerator, *denominator;
public:
  Division(Expression *numerator, Expression *denominator);
  bool CanonicalEqualToSameType(Expression *other);
  bool CanonicalSmallerThanSameType(Expression *other);
  double operator()(double x) const;
  void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
  Expression *diff() const;
  Expression *clone() const;
  ~Division();
  virtual Expression *TrySimplifyAdding(Expression *right);
  virtual Expression *TrySimplifyMultiplying(Expression *right);
  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed);
};

class Composition: public Expression {
  Expression *left, *right;
public:
  Composition(Expression *left, Expression *right);
  bool CanonicalEqualToSameType(Expression *other);
  bool CanonicalSmallerThanSameType(Expression *other);

  double operator()(double x) const {
    return (*left)((*right)(x));
  }

  void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
  Expression *diff() const;
  Expression *clone() const;

  ~Composition() {
    delete left;
    delete right;
  }

  virtual Expression *TrySimplifyAdding(Expression *right);
  virtual Expression *TrySimplifyMultiplying(Expression *right);
  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed);
};

class Constant: public Expression {
  double c;
public:
  Constant(double c) : Expression(TypeConstant), c(c) {
  }

  double value() const { return c; }

  bool CanonicalEqualToSameType(Expression *other);
  bool CanonicalSmallerThanSameType(Expression *other);

  double operator()(double x) const {
    return c;
  }

  void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;

  Expression *diff() const {
    return new Constant(0);
  }

  Expression *clone() const {
    return new Constant(*this);
  }

  virtual Expression *TrySimplifyAdding(Expression *right);
  virtual Expression *TrySimplifyMultiplying(Expression *right);

  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed) { return NULL; }
};

class VariableX: public Expression {
public:
  VariableX() : Expression(TypeVariable) { }

  bool CanonicalEqualToSameType(Expression *other) { return true; }

  bool CanonicalSmallerThanSameType(Expression *other) { return false; }

  double operator()(double x) const { return x; }

  void recursivePrint(std::string &output, OperatorPrecedence::Order order) const {
    output.push_back('x');
  }

  Expression *diff() const { return new Constant(1); }

  Expression *clone() const { return new VariableX; }

  virtual Expression *TrySimplifyAdding(Expression *right);
  virtual Expression *TrySimplifyMultiplying(Expression *right);

  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed) { return NULL; }
};

class Polynomial: public Expression {
  std::vector<double> para;
public:
  Polynomial(const std::vector<double> &parametre);
  Polynomial(double a, double b);
  bool CanonicalEqualToSameType(Expression *other);
  bool CanonicalSmallerThanSameType(Expression *other);
  double operator()(double x) const;
  void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
  Expression *diff() const;
  Expression *clone() const;

  std::vector<double> getParameter() const { return para; }

  virtual Expression *TrySimplifyAdding(Expression *right);
  virtual Expression *TrySimplifyMultiplying(Expression *right);

  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed) { return NULL; }

};

class ElementryFunction: public Expression {
public:
  ElementryFunction(NodeType type) : Expression(type) {
  }

  void recursivePrint(std::string &output, OperatorPrecedence::Order order) const;
  // used in the case of composition, such as sin( cos(x) )
  void compositionPrint(std::string &output, const Expression *innerFunction) const;
  virtual std::string functionName() const = 0;
};

class Trigo: public ElementryFunction {
public:
  enum TrigoType {
    Sin, Cos, Tan
  };
private:
  TrigoType trigoType;
public:
  Trigo(TrigoType trigoType) : ElementryFunction(TypeTrigo), trigoType(trigoType) {
  }

  bool CanonicalEqualToSameType(Expression *other);
  bool CanonicalSmallerThanSameType(Expression *other);
  virtual Expression *diff() const;
  virtual Expression *clone() const;
  virtual double operator()(double x) const;
  std::string functionName() const;

  virtual Expression *TrySimplifyAdding(Expression *right) { return NULL; }

  virtual Expression *TrySimplifyMultiplying(Expression *right) { return NULL; }

  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed) { return NULL; }
};

class Logarithm: public ElementryFunction {
public:
  Logarithm() : ElementryFunction(TypeLog) {
  }

  bool CanonicalEqualToSameType(Expression *other) {
    return true;
  }

  bool CanonicalSmallerThanSameType(Expression *other) {
    return false;
  }

  virtual Expression *diff() const;
  virtual Expression *clone() const;
  virtual double operator()(double x) const;
  std::string functionName() const;
  virtual Expression *TrySimplifyAdding(Expression *right);
  virtual Expression *TrySimplifyMultiplying(Expression *right);
  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed);
};

class Exponential: public ElementryFunction {
public:
  Exponential() : ElementryFunction(TypeExp) {
  }

  bool CanonicalEqualToSameType(Expression *other) {
    return true;
  }

  bool CanonicalSmallerThanSameType(Expression *other) {
    return false;
  }

  virtual Expression *diff() const;
  virtual Expression *clone() const;
  virtual double operator()(double x) const;
  std::string functionName() const;
  virtual Expression *TrySimplifyAdding(Expression *right);
  virtual Expression *TrySimplifyMultiplying(Expression *right);
  // virtual Expression *TrySimplifyDivided(Expression *right) const = 0;
  // virtual Expression *TrySimplifyDividing(Expression *left) const = 0;
  // virtual Expression *TrySimplifyPowered(Expression *right) const = 0;
  // virtual Expression *TrySimplifyPowering(Expression *left) const = 0;
  // virtual Expression *TrySimplifyComposed(Expression *right) const = 0;
  // virtual Expression *TrySimplifyComposing(Expression *left) const = 0;
  virtual Expression *simplify(bool &changed);
};


#endif // FUNCTION_H
