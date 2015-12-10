#include <iostream>
#include <cmath>
#include "function.h"
#include "ExpressionEvaluator.h"

using namespace std;

double newtonMethod(Expression *f, double x0, double target) {
  Expression *df = f->diffSimplify();
  double xn = x0;
  double lastXn;
  for (int iter = 0; iter < 100; iter++) {
    lastXn = xn;
    xn = xn + (target - (*f)(xn)) / (*df)(xn);
    if (abs(lastXn - xn) < 1e-10)
      break;
  }
  delete df;
  return xn;
}
double solve(std::string equation, double x0, double target) {
  ExpressionEvaluator evaluator;
  Expression *e1;
  e1 = evaluator.evaluate(equation);
  double answer = newtonMethod(e1, x0, target);
  cout << e1->stringPrint() << "==" << target << ", x=" << answer <<
      "\tverify:" << e1->stringPrint() << ", x=" << answer << ", =" << (*e1)(answer) << endl;
  delete e1;
}
void simplifyTest(std::string expression) {
  ExpressionEvaluator evaluator;
  Expression *e;
  e = evaluator.evaluate(expression);
  Expression *df = e->diff();
  Expression *dfsimple = e->diffSimplify();
  cout << "String:\t" << expression << endl;
  cout << "Evaluation:\t" << e->stringPrint() << endl;
  cout << "Diff:\t" << df->stringPrint() << endl;
  cout << "Df,Simplyfied:\t" << dfsimple->stringPrint() << endl;
  cout<<endl;
  delete e;
  delete df;
  delete dfsimple;
}
int main() {
  cout << "simplify test:" << endl << endl;
  simplifyTest("x*x");
  simplifyTest("x*x*sin(x)");
  simplifyTest("sin(x)/x+cos(x)/x");
  simplifyTest("sin(cos(x))");

  cout << endl << "newton method test:" << endl << endl;
  solve("x*x*x", 10, 27);
  solve("x*x", 10, 64);
  solve("sin(x)", 0.1, 1);
  solve("cos(x)", 0.5, 0);
  solve("sin(x)/x+cos(x)*x/3", 0.5, 0);
  return 0;
}
