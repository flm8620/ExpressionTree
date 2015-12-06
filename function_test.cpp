#include <cmath>
#include <vector>
#include <string>
#include "catch.hpp"
#include "function.h"
#include "ExpressionEvaluator.h"
using namespace std;
TEST_CASE("Trigo functions", "[funtion][trigo]") {
  Expression *Esin = new Trigo(Trigo::Sin);
  Expression *Ecos = new Trigo(Trigo::Cos);
  Expression *Etan = new Trigo(Trigo::Tan);
  Expression *d;
  REQUIRE((*Esin).stringPrint() == string("sin(x)"));
  REQUIRE((*Ecos).stringPrint() == string("cos(x)"));
  REQUIRE((*Etan).stringPrint() == string("tan(x)"));
  REQUIRE((*Esin)(1.23) == sin(1.23));
  REQUIRE((*Ecos)(1.23) == cos(1.23));
  REQUIRE((*Etan)(1.23) == tan(1.23));
  d = Esin->diff();
  REQUIRE((*d).stringPrint() == string("cos(x)"));
  delete d;
  d = Ecos->diff();
  REQUIRE((*d).stringPrint() == string("-1*sin(x)"));
  delete d;
  d = Etan->diff();
  REQUIRE((*d).stringPrint() == string("1/(cos(x)*cos(x))"));
  REQUIRE((*d)(1.23) == Approx(1 / (cos(1.23) * cos(1.23))));
  delete d;
  delete Esin;
  delete Ecos;
  delete Etan;
}

TEST_CASE("Constant", "[function][const]") {
  Expression *E1 = new Constant(1);
  Expression *E2 = new Constant(3);
  Expression *E3 = new Constant(-4);
  REQUIRE(E1->stringPrint() == "1");
  REQUIRE(E2->stringPrint() == "3");
  REQUIRE(E3->stringPrint() == "-4");
  Expression *d = E1->diff();
  REQUIRE(d->stringPrint() == "0");
  REQUIRE((*d)(1.23) == 0);
  delete d;
  delete E1;
  delete E2;
  delete E3;
}

TEST_CASE("log and exp") {
  Expression *ln = new Logarithm;
  Expression *eexp = new Exponential;
  Expression *dln = ln->diff();
  Expression *dexp = eexp->diff();
  REQUIRE(ln->stringPrint() == "ln(x)");
  REQUIRE(dln->stringPrint() == "1/x");
  REQUIRE(eexp->stringPrint() == "exp(x)");
  REQUIRE(dexp->stringPrint() == "exp(x)");
  delete ln;
  delete eexp;
  delete dln;
  delete dexp;
}
TEST_CASE("polynomial") {
  vector<double> a;
  a.push_back(1);
  Expression *poly;
  a.push_back(-2);
  poly = new Polynomial(a);
  REQUIRE(poly->stringPrint() == "Poly[1-2x]");
  delete poly;
  a.push_back(3);
  poly = new Polynomial(a);
  REQUIRE(poly->stringPrint() == "Poly[1-2x+3x^2]");
  delete poly;
  a.push_back(0);
  poly = new Polynomial(a);
  REQUIRE(poly->stringPrint() == "Poly[1-2x+3x^2]");
  delete poly;
  a.push_back(5.1);
  poly = new Polynomial(a);
  REQUIRE(poly->stringPrint() == "Poly[1-2x+3x^2+5.1x^4]");
  delete poly;
}

TEST_CASE("operator") {
  Expression *e1 = new Constant(2.1);
  Expression *e2 = new Trigo(Trigo::Sin);
  Expression *ep = new Addition(e1, e2);
  REQUIRE(ep->stringPrint() == "2.1+sin(x)");
  delete ep;//e1 e2 deleted
  e1 = new Constant(2.1);
  e2 = new Trigo(Trigo::Sin);
  ep = new Addition(e2, e1);
  REQUIRE(ep->stringPrint() == "2.1+sin(x)");
  delete ep;//e1 e2 deleted
  e1 = new Constant(2.1);
  e2 = new Trigo(Trigo::Sin);
  Expression *e3 = new VariableX;
  ep = new Addition(e2, e1);
  Expression *em = new Multiplication(e3, ep);
  REQUIRE(em->stringPrint() == "x*(2.1+sin(x))");
  delete em;
  e1 = new Constant(2.1);
  e2 = new Trigo(Trigo::Sin);
  e3 = new Trigo(Trigo::Sin);
  ep = new Division(e1, e2);
  em = new Composition(e3, ep);
  REQUIRE(em->stringPrint() == "sin(2.1/sin(x))");
  delete em;
}

TEST_CASE("ExpressionEvaluator") {
  ExpressionEvaluator evaluator;
  Expression *e1;
  e1 = evaluator.evaluate("1");
  REQUIRE(e1->stringPrint() == "1");
  delete e1;
  e1 = evaluator.evaluate("1+2");
  REQUIRE(e1->stringPrint() == "3");
  delete e1;
  e1 = evaluator.evaluate("-1");
  REQUIRE(e1->stringPrint() == "-1");
  delete e1;
  e1 = evaluator.evaluate("-3+1");
  REQUIRE(e1->stringPrint() == "-2");
  delete e1;
  e1 = evaluator.evaluate("-1+1");
  REQUIRE(e1->stringPrint() == "0");
  delete e1;
  e1 = evaluator.evaluate("1+2+3");
  REQUIRE(e1->stringPrint() == "6");
  delete e1;
  e1 = evaluator.evaluate("1+(2+3)");
  REQUIRE(e1->stringPrint() == "6");
  delete e1;
  e1 = evaluator.evaluate("1+2-3");
  REQUIRE(e1->stringPrint() == "0");
  delete e1;
  e1 = evaluator.evaluate("-1-2-3.5");
  REQUIRE(e1->stringPrint() == "-6.5");
  delete e1;
  e1 = evaluator.evaluate("-1-x+1");
  REQUIRE(e1->stringPrint() == "Poly[-x]");
  delete e1;
  e1 = evaluator.evaluate("1*2*3*4*5");
  REQUIRE(e1->stringPrint() == "120");
  delete e1;
  e1 = evaluator.evaluate("2*x*3");
  REQUIRE(e1->stringPrint() == "Poly[6x]");
  delete e1;
  e1 = evaluator.evaluate("2*x*x+3*x");
  REQUIRE(e1->stringPrint() == "Poly[3x+2x^2]");
  delete e1;
  e1 = evaluator.evaluate("(2*x*x+3*x)*x");
  REQUIRE(e1->stringPrint() == "Poly[3x^2+2x^3]");
  delete e1;
  e1 = evaluator.evaluate("1/x/x");
  REQUIRE(e1->stringPrint() == "1/Poly[x^2]");
  delete e1;
  e1 = evaluator.evaluate("1/x/x/x/x/x");
  REQUIRE(e1->stringPrint() == "1/Poly[x^5]");
  delete e1;
  e1 = evaluator.evaluate("1/x/-x/x/x/x");
  REQUIRE(e1->stringPrint() == "1/Poly[-x^5]");
  delete e1;
  e1 = evaluator.evaluate("x/3-x/3");
  REQUIRE(e1->stringPrint() == "0");
  delete e1;
  e1 = evaluator.evaluate("x/1");
  REQUIRE(e1->stringPrint() == "x");
  delete e1;
  e1 = evaluator.evaluate("(2/x)/(sin(x)/exp(x))");
  REQUIRE(e1->stringPrint() == "(2*exp(x))/(x*sin(x))");
  delete e1;
  e1 = evaluator.evaluate("sin(x)/x+cos(x)/x");
  REQUIRE(e1->stringPrint() == "(sin(x)+cos(x))/x");
  delete e1;
  e1 = evaluator.evaluate("sin(x)-sin(x)");
  REQUIRE(e1->stringPrint() == "0");
  delete e1;
  e1 = evaluator.evaluate("sin(x)*x*x-sin(x)*x*x*x/x");
  REQUIRE((*e1)(1.234) == Approx(0));
  delete e1;
}
TEST_CASE("Diff") {
  ExpressionEvaluator evaluator;
  Expression *e1, *d;
  e1 = evaluator.evaluate("1");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "0");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("x");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "1");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("x+x*x");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "Poly[1+2x]");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("x*sin(x)");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "x*cos(x)+sin(x)");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("sin(x)/x");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "(-1*sin(x)+x*cos(x))/Poly[x^2]");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("2-x");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "-1");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("2/3");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "0");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("log(x)");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "1/x");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("sin(x)");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "cos(x)");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("cos(x)");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "-1*sin(x)");
  delete e1;
  delete d;
  e1 = evaluator.evaluate("tan(x)");
  d = e1->diffSimplify();
  REQUIRE(d->stringPrint() == "1/(cos(x)*cos(x))");
  delete e1;
  delete d;

}