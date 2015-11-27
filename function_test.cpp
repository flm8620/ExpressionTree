#include <cmath>
#include <vector>
#include <string>
#include "catch.hpp"
#include "function.h"
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
    REQUIRE((*d).stringPrint() == string("-sin(x)"));
    delete d;
    d = Etan->diff();
    REQUIRE((*d).stringPrint() == string("1/(cos(x)*cos(x))"));
    REQUIRE((*d)(1.23) == Approx(1/(cos(1.23)*cos(1.23))));
    delete d;
    delete Esin;
    delete Ecos;
    delete Etan;
}

TEST_CASE("Constant", "[function][const]"){
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
    delete E1, E2, E3;
}

TEST_CASE("log and exp"){
    Expression *ln = new Logarithm;
    Expression *eexp = new Exponential;
    Expression *dln = ln->diff();
    Expression *dexp = eexp->diff();
    REQUIRE(ln->stringPrint() == "ln(x)");
    REQUIRE(dln->stringPrint() == "1/x");
    REQUIRE(eexp->stringPrint() == "exp(x)");
    REQUIRE(dexp->stringPrint() == "exp(x)");
    delete ln, eexp, dln, dexp;
}
TEST_CASE("polynomial"){
    vector<double> a;
    a.push_back(1);
    Expression *poly;
    a.push_back(-2);
    poly = new Polynome(a);
    REQUIRE(poly->stringPrint()=="Poly[1-2x]");
    delete poly;
    a.push_back(3);
    poly = new Polynome(a);
    REQUIRE(poly->stringPrint() == "Poly[1-2x+3x^2]");
    delete poly;
    a.push_back(0);
    poly = new Polynome(a);
    REQUIRE(poly->stringPrint() == "Poly[1-2x+3x^2]");
    delete poly;
    a.push_back(5.1);
    poly = new Polynome(a);
    REQUIRE(poly->stringPrint() == "Poly[1-2x+3x^2+5.1x^4]");
    delete poly;
    poly=new Affine(2,3.1);
    REQUIRE(poly->stringPrint()=="Poly[3.1+2x]");
    delete poly;
}

TEST_CASE("operator"){
    Expression* e1=new Constant(2.1);
    Expression* e2=new Trigo(Trigo::Sin);
    Expression* ep=new Operator(Operator::Add,e1,e2);
    REQUIRE(ep->stringPrint()=="2.1+sin(x)");
    delete ep;//e1 e2 deleted
    e1=new Constant(2.1);
    e2=new Trigo(Trigo::Sin);
    ep=new Operator(Operator::Sub,e2,e1);
    REQUIRE(ep->stringPrint()=="sin(x)-2.1");
    delete ep;//e1 e2 deleted
    e1=new Constant(2.1);
    e2=new Trigo(Trigo::Sin);
    Expression *e3=new VariableX;
    ep=new Operator(Operator::Sub,e2,e1);
    Expression *em=new Operator(Operator::Multi,e3,ep);
    REQUIRE(em->stringPrint()=="x*(sin(x)-2.1)");
    delete em;
    e1=new Constant(2.1);
    e2=new Trigo(Trigo::Sin);
    e3=new Trigo(Trigo::Sin);
    ep=new Operator(Operator::Divide,e1,e2);
    em=new Operator(Operator::Composition,e3,ep);
    REQUIRE(em->stringPrint()=="sin(2.1/sin(x))");
    delete em;

}
