//
// Created by FLM on 2015/12/5 0005.
//

#ifndef EXPRESSIONEVALUATOR_H
#define EXPRESSIONEVALUATOR_H
#include "function.h"

enum FunctionName {
  Sin, Cos, Tan, Exp, Log,
};

struct Symbol {
  enum Type {
    Add, Sub,
    Multi, Divide,
    Power,
    Positive, Negative,
    LeftParenthese,
    RightParenthese,
    FunName,
    Number,
    theVariableX,
    ApplyFunction,
  } type;
  double number;
  FunctionName funName;

  Symbol(Type type) : type(type) { assert(type != FunName && type != Number); }

  Symbol(double x) : number(x), type(Number) { }

  Symbol(FunctionName name) : funName(name), type(FunName) { }

  OperatorPrecedence::Order precedence();
};

class ExpressionEvaluator {
 private:
  std::string expression;
  int pos;
  std::queue<Symbol> symbolQueue;
  std::queue<Symbol> symbolPolish;
  void stringDecomposition();
  void reversePolishNotation();
  Expression *constructTree();
 public:
  Expression *evaluate(const std::string &s);
};

#endif //EXPRESSIONEVALUATOR_H
