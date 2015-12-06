//
// Created by FLM on 2015/12/5 0005.
//

#include "ExpressionEvaluator.h"
#include <sstream>
#include <stdexcept>

using namespace std;

OperatorPrecedence::Order Symbol::precedence() {
  switch (this->type) {
    case Add:
    case Sub:
      return OperatorPrecedence::AddSub;
    case Multi:
    case Divide:
      return OperatorPrecedence::MultiDivide;
    case Power:
      return OperatorPrecedence::Power;
    case Positive:
    case Negative:
      return OperatorPrecedence::PositiveNegative;
    case ApplyFunction:
      return OperatorPrecedence::Composition;
    default:
      assert(false);
  }
}

void ExpressionEvaluator::stringDecomposition() {
  stringstream ss;
  ss << this->expression;
  char c;
  while ((c = ss.peek()) != EOF) {
    if (c == ' ') {
      ss.get();
      continue;
    } else if ((c >= '0' && c <= '9') || c == '.') {
      double x;
      ss >> x;
      if (ss.fail()) throw invalid_argument("invalid expression");
      symbolQueue.push(Symbol(x));
    } else if (c == '+') {
      ss.get();
      if (symbolQueue.size() == 0) {
        symbolQueue.push(Symbol(Symbol::Positive));
      } else {
        switch (symbolQueue.back().type) {
          case Symbol::FunName:
          case Symbol::Number:
          case Symbol::theVariableX:
          case Symbol::RightParenthese:
            symbolQueue.push(Symbol(Symbol::Add));
            break;
          default:
            symbolQueue.push(Symbol(Symbol::Positive));
            break;
        }
      }
    } else if (c == '-') {
      ss.get();
      if (symbolQueue.size() == 0) {
        symbolQueue.push(Symbol(Symbol::Negative));
      } else {
        switch (symbolQueue.back().type) {
          case Symbol::FunName:
          case Symbol::Number:
          case Symbol::theVariableX:
          case Symbol::RightParenthese:
            symbolQueue.push(Symbol(Symbol::Sub));
            break;
          default:
            symbolQueue.push(Symbol(Symbol::Negative));
            break;
        }
      }
    } else if (c == '*') {
      ss.get();
      symbolQueue.push(Symbol(Symbol::Multi));
    } else if (c == '/') {
      ss.get();
      symbolQueue.push(Symbol(Symbol::Divide));
    } else if (c == '^') {
      ss.get();
      symbolQueue.push(Symbol(Symbol::Power));
    } else if (c == '(') {
      ss.get();
      symbolQueue.push(Symbol(Symbol::LeftParenthese));
    } else if (c == ')') {
      ss.get();
      symbolQueue.push(Symbol(Symbol::RightParenthese));
    } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
      // identifiers
      string name;
      char next;
      do {// get full name string
        name.push_back(ss.get());
        next = ss.peek();
      } while ((next >= 'a' && next <= 'z')
          || (next >= 'A' && next <= 'Z')
          || (next >= '0' && next <= '9')
          || next == '_');
      if (name == "sin")
        symbolQueue.push(Symbol(Sin));
      else if (name == "cos")
        symbolQueue.push(Symbol(Cos));
      else if (name == "tan")
        symbolQueue.push(Symbol(Tan));
      else if (name == "exp")
        symbolQueue.push(Symbol(Exp));
      else if (name == "log")
        symbolQueue.push(Symbol(Log));
      else if (name == "x")
        symbolQueue.push(Symbol(Symbol::theVariableX));
      else
        throw invalid_argument("invalid function name");
    } else {
      throw invalid_argument("invalid symbol");
    }
  }

}

void ExpressionEvaluator::reversePolishNotation() {
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
      case Symbol::Multi:
      case Symbol::Divide:
      case Symbol::Positive:
      case Symbol::Negative:
      case Symbol::Power:
      case Symbol::ApplyFunction:
        while (!operators.empty() && operators.top().type != Symbol::LeftParenthese) {
          Symbol top = operators.top();
          if (top.precedence() < sym.precedence())break;
          symbolPolish.push(operators.top());
          operators.pop();
        }
        operators.push(sym);
        break;
      case Symbol::RightParenthese:
        while (!operators.empty()
            && operators.top().type != Symbol::LeftParenthese) {
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
  } // while
  while (!operators.empty()) {
    if (operators.top().type == Symbol::LeftParenthese)
      throw invalid_argument("a ')' is missed in the expression");
    assert(operators.top().type != Symbol::RightParenthese);
    symbolPolish.push(operators.top());
    operators.pop();
  }
}

Expression *ExpressionEvaluator::constructTree() {
  stack<Expression *> exprStack;
  while (!symbolPolish.empty()) {
    Symbol sym = symbolPolish.front();
    symbolPolish.pop();
    switch (sym.type) {
      case Symbol::RightParenthese:
      case Symbol::LeftParenthese:
        //no parenthesis is possible in polish notation
        assert(false);
        break;
      case Symbol::Number:
        exprStack.push(new Constant(sym.number));
        break;
      case Symbol::Add: {
        assert(exprStack.size() >= 2);
        Expression *b = exprStack.top();
        exprStack.pop();
        Expression *a = exprStack.top();
        exprStack.pop();
        Expression *ab = new Addition(a, b);
        exprStack.push(ab);
        break;
      }
      case Symbol::Sub: {
        assert(exprStack.size() >= 2);
        Expression *b = exprStack.top();
        exprStack.pop();
        Expression *a = exprStack.top();
        exprStack.pop();
        Expression *ab = new Addition(a, new Multiplication(new Constant(-1), b));
        exprStack.push(ab);
        break;
      }
      case Symbol::Multi: {
        assert(exprStack.size() >= 2);
        Expression *b = exprStack.top();
        exprStack.pop();
        Expression *a = exprStack.top();
        exprStack.pop();
        Expression *ab = new Multiplication(a, b);
        exprStack.push(ab);
        break;
      }
      case Symbol::Divide: {
        assert(exprStack.size() >= 2);
        Expression *b = exprStack.top();
        exprStack.pop();
        Expression *a = exprStack.top();
        exprStack.pop();
        Expression *ab = new Division(a, b);
        exprStack.push(ab);
        break;
      }
      case Symbol::FunName: {
        switch (sym.funName) {
          case Sin:
            exprStack.push(new Trigo(Trigo::Sin));
            break;
          case Cos:
            exprStack.push(new Trigo(Trigo::Cos));
            break;
          case Tan:
            exprStack.push(new Trigo(Trigo::Tan));
            break;
          case Log:
            exprStack.push(new Logarithm);
            break;
          case Exp:
            exprStack.push(new Exponential);
            break;
        }
        break;
      }
      case Symbol::theVariableX:
        exprStack.push(new VariableX);
        break;
      case Symbol::ApplyFunction: {
        assert(exprStack.size() >= 2);
        Expression *x = exprStack.top();
        exprStack.pop();
        Expression *f = exprStack.top();
        exprStack.pop();
        Expression *fx = new Composition(f, x);
        exprStack.push(fx);
        break;
      }
      case Symbol::Positive:
        break;
      case Symbol::Negative: {
        assert(exprStack.size() >= 1);
        Expression *a = exprStack.top();
        exprStack.pop();
        Expression *minus_a = new Multiplication(new Constant(-1), a);
        exprStack.push(minus_a);
        break;
      }
    }
  }
  assert(exprStack.size() == 1);
  Expression *e = exprStack.top();
  bool changed;
  Expression *simplify = e->simplify(changed);
  if (simplify)
    return simplify;
  else
    return e;
}

Expression *ExpressionEvaluator::evaluate(const std::string &s) {
  pos = 0;
  //clear old data
  while (!symbolPolish.empty())
    symbolPolish.pop();
  while (!symbolQueue.empty())
    symbolQueue.pop();

  if (s.size() == 0) throw invalid_argument("empty string");
  expression = s;
  stringDecomposition();
  reversePolishNotation();
  return constructTree();
}
