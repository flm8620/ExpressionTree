#include "function.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>

using namespace std;

bool Expression::CanonicalEqualTo(Expression *other) {
  if (this->nodeType() != other->nodeType()) return false;
  return this->CanonicalEqualToSameType(other);
}

bool Expression::CanonicalSmallerThan(Expression *other) {
  if (this->nodeType() < other->nodeType())
    return true;
  else if (this->nodeType() > other->nodeType())
    return false;
  else
    return this->CanonicalSmallerThanSameType(other);
}

string Expression::stringPrint() const {
  string s;
  recursivePrint(s, OperatorPrecedence::None);
  return s;
}

void CommutativeOperators::recursivePrintCommutative(string &output,
                                                     OperatorPrecedence::Order order,
                                                     OperatorPrecedence::Order selfOrder,
                                                     char symbol) const {
  bool closeParenthese = false;
  OperatorPrecedence::Order nextOrder;
  if (order >= selfOrder) {
    output.push_back('(');
    closeParenthese = true;
  }
  nextOrder = selfOrder;
  auto it = childrenSet.begin();
  (*it)->recursivePrint(output, nextOrder);
  it++;
  for (; it != childrenSet.end(); it++) {
    output.push_back(symbol);
    (*it)->recursivePrint(output, nextOrder);
  }
  if (closeParenthese) output.push_back(')');
}

void CommutativeOperators::construct(ExpressionSet children) {
  for (auto i = children.begin(); i != children.end(); i++)
    if (*i == 0) throw invalid_argument("Null pointers in arguments");
  this->childrenSet = children;
}

void CommutativeOperators::construct(Expression *a, Expression *b) {
  ExpressionSet two;
  two.insert(a);
  two.insert(b);
  construct(two);
}

bool CommutativeOperators::simplifyChildren() {
  bool needContinue = true;
  bool changed = false;
  while (needContinue) {
    needContinue = false;
    for (auto i = childrenSet.begin(); i != childrenSet.end(); i++) {
      bool childChanged;
      Expression *simplified = (*i)->simplify(childChanged);
      if (simplified) {
        changed = true;
        needContinue = true;
        delete *i;
        i = childrenSet.erase(i);
        childrenSet.insert(simplified);
      } else if (childChanged) {
        changed = true;
        Expression *afterChange = *i;
        i = childrenSet.erase(i);
        childrenSet.insert(afterChange);
      }
    }
  }
  return changed;
}

bool CommutativeOperators::CanonicalEqualToSameType(Expression *other) {
  assert(other->nodeType() == TypeAdd || other->nodeType() == TypeMulti);
  CommutativeOperators *p = static_cast<CommutativeOperators *>(other);
  int n = childrenSet.size();
  int m = p->childrenSet.size();
  if (n != m) {
    return false;
  } else {
    auto i = childrenSet.begin();
    auto j = p->childrenSet.begin();
    for (; i != childrenSet.end(); i++, j++) {
      if (!(*i)->CanonicalEqualTo(*j))
        return false;
    }
    return true;
  }
}

bool CommutativeOperators::CanonicalSmallerThanSameType(Expression *other) {
  assert(other->nodeType() == TypeAdd || other->nodeType() == TypeMulti);
  CommutativeOperators *p = static_cast<CommutativeOperators *>(other);
  int n = childrenSet.size();
  int m = p->childrenSet.size();
  if (n < m) {
    return true;
  } else if (n > m) {
    return false;
  } else {
    auto i = childrenSet.begin();
    auto j = p->childrenSet.begin();
    for (; i != childrenSet.end(); i++, j++) {
      if ((*i)->CanonicalSmallerThan(*j))
        return true;
    }
    return false;
  }
}

Expression *CommutativeOperators::clone() const {
  ExpressionSet cl;
  for (auto i = childrenSet.begin(); i != childrenSet.end(); i++)
    cl.insert((*i)->clone());
  return new Addition(cl);
}

CommutativeOperators::~CommutativeOperators() {
  for (auto i = childrenSet.begin(); i != childrenSet.end(); i++)
    delete *i;
}

double Addition::operator()(double x) const {
  double sum = 0;
  for (auto it = childrenSet.begin(); it != childrenSet.end(); it++)
    sum += (**it)(x);
  return sum;
}

void Addition::recursivePrint(string &output, OperatorPrecedence::Order order) const {
  recursivePrintCommutative(output, order, OperatorPrecedence::AddSub, '+');
}

Expression *Addition::diff() const {
  multiset<Expression *, ExpressionComparator> d;
  for (auto it = childrenSet.begin(); it != childrenSet.end(); it++)
    d.insert((*it)->diff());
  return new Addition(d);
}

Expression *Addition::TrySimplifyAdding(Expression *right) {
  assert(false);//must have been treated in Addition::simplify()
  return nullptr;
}

Expression *Addition::TrySimplifyMultiplying(Expression *right) {
  return nullptr;
}

Expression *Addition::simplify(bool &changed) {
  bool needContinue = true;
  changed = false;
  while (needContinue) {
    needContinue = false;
    if (this->simplifyChildren()) {
      changed = true;
      //needContinue = true;
    }
    for (auto it = childrenSet.begin(); it != childrenSet.end(); it++) {
      switch ((*it)->nodeType()) {
        case TypeAdd: {
          Addition *p = static_cast<Addition *>(*it);
          //pointer ownership changed
          childrenSet.insert(p->childrenSet.begin(), p->childrenSet.end());
          p->childrenSet.clear();
          delete *it;
          it = childrenSet.erase(it);
          changed = true;
          needContinue = true;
          break;
        }
        case TypeConstant: {
          Constant *p = static_cast<Constant *>(*it);
          if (p->value() == 0) {
            changed = true;
            needContinue = true;
            delete *it;
            it = childrenSet.erase(it);
          }
          break;
        }
        default:
          break;
      }

      if (it == childrenSet.end())
        continue;
      auto next = it;
      next++;
      if (next == childrenSet.end())
        continue;

      //simplify two by two
      for (; next != childrenSet.end(); next++) {
        assert(it!=next);
        if ((*it)->CanonicalEqualTo(*next)) {
          //a+a=2*a
          changed = true;
          needContinue = true;
          Expression *r = new Multiplication(new Constant(2), (*it)->clone());
          delete *it;
          delete *next;
          childrenSet.insert(r);
          childrenSet.erase(it);
          it = childrenSet.erase(next);
          break;
        } else {
          Expression *p = (*it)->TrySimplifyAdding(*next);
          if (p) {
            changed = true;
            needContinue = true;
            delete *it;
            delete *next;
            childrenSet.insert(p);
            childrenSet.erase(it);
            it = childrenSet.erase(next);
            break;
          } else {
            Expression *pp = (*next)->TrySimplifyAdding(*it);
            if (pp) {
              changed = true;
              needContinue = true;
              delete *it;
              delete *next;
              childrenSet.insert(pp);
              childrenSet.erase(it);
              it = childrenSet.erase(next);
              break;
            }
          }
        }

      }

    }
  }
  if (childrenSet.size() == 0) {
    return new Constant(0);
  } else {
    return NULL;
  }
}

double Multiplication::operator()(double x) const {
  double prod = 1;
  for (auto it = childrenSet.begin(); it != childrenSet.end(); it++)
    prod *= (**it)(x);
  return prod;
}

void Multiplication::recursivePrint(string &output, OperatorPrecedence::Order order) const {
  recursivePrintCommutative(output, order, OperatorPrecedence::MultiDivide,
                            '*');
}

Expression *Multiplication::diff() const {
  ExpressionSet term;
  ExpressionSet sum;
  for (auto k = childrenSet.begin(); k != childrenSet.end(); k++) {
    for (auto i = childrenSet.begin(); i != childrenSet.end(); i++) {
      if (i == k)
        term.insert((*i)->diff());
      else
        term.insert((*i)->clone());
    }
    sum.insert(new Multiplication(term));
    term.clear();
  }
  return new Addition(sum);
}

Expression *Multiplication::TrySimplifyAdding(Expression *right) {
  // a*b*c+a*b*d = a*b*(c+d)
  // a*b*c*d +  b*d = (a*c+1)*(b*d)


  // find common elements in this->childrenSet and right->childrenSet
  // this:   a  *c*d*e  *g
  // right:    b*c  *e*f*g
  // common:     c  *e  *g
  // A:      a    *d        // if NULL, A=1
  // B:        b      *f    // if NULL, B=1
  // simplify: (A+B)*common = (a*d+b*f)*c*e*g

  // !!! A,B,commonElements don't have ownership of their elements
  ExpressionSet A, B, commonElements;
  A = this->childrenSet;
  if (right->nodeType() == TypeMulti) {
    Multiplication *p = static_cast<Multiplication *>(right);
    B = p->childrenSet;
  } else {
    // case of: a*b*c + c
    B.insert(right);
  }
  auto i = A.begin();
  auto j = B.begin();
  while (i != A.end() && j != B.end()) {
    if ((*i)->CanonicalSmallerThan(*j)) {
      i++;
    } else if ((*i)->CanonicalEqualTo(*j)) {
      commonElements.insert(*i);
      i = A.erase(i);
      j = B.erase(j);
    } else {
      j++;
    }
  }
  // case of a*b + c
  if (commonElements.empty())
    return NULL;
  // case of this==right should be already treated in Addition: a+a=2*a
  assert(!(A.empty() && B.empty()));
  // now, A,B,commonElements have the ownership of their elements
  for (auto item : A) {
    item = item->clone();
  }
  for (auto item : B) {
    item = item->clone();
  }
  for (auto item : commonElements) {
    item = item->clone();
  }
  Expression *multiA;
  Expression *multiB;
  if (A.empty()) {
    multiA = new Constant(1);
    multiB = new Multiplication(B);
  } else if (B.empty()) {
    multiB = new Constant(1);
    multiA = new Multiplication(A);
  }
  Expression *AplusB = new Addition(multiA, multiB);
  commonElements.insert(AplusB);
  Expression *final = new Multiplication(commonElements);
  bool changed;
  Expression *finalSimplified = final->simplify(changed);
  if (finalSimplified)
    return finalSimplified;
  else
    return final;
}

Expression *Multiplication::TrySimplifyMultiplying(Expression *right) {
  //must have been treated in Multiplication::simplify()
  return nullptr;
}

Expression *Multiplication::simplify(bool &changed) {
  bool needContinue = true;
  while (needContinue) {
    needContinue = false;
    if (this->simplifyChildren()) {
      changed = true;
      //needContinue = true;
    }
    for (auto it = childrenSet.begin(); it != childrenSet.end(); it++) {
      switch ((*it)->nodeType()) {
        case TypeMulti: {
          Multiplication *p = static_cast<Multiplication *>(*it);
          childrenSet.insert(p->childrenSet.begin(), p->childrenSet.end());
          p->childrenSet.clear();
          delete p;
          changed = true;
          needContinue = true;
          break;
        }
        case TypeConstant: {
          Constant *p = static_cast<Constant *>(*it);
          if (p->value() == 1) {
            changed = true;
            needContinue = true;
            delete *it;
            it = childrenSet.erase(it);
          } else if (p->value() == 0) {
            childrenSet.clear();
            changed = true;
            needContinue = false;
          }
        }
      }

      if (it == childrenSet.end())
        continue;
      auto next = it;
      next++;
      if (next == childrenSet.end())
        continue;
      //simplify two by two
      for (; next != childrenSet.end(); next++) {
        Expression *p = (*it)->TrySimplifyMultiplying(*next);
        if (p) {
          changed = true;
          needContinue = true;
          delete *it;
          delete *next;
          childrenSet.insert(p);
          childrenSet.erase(it);
          it = childrenSet.erase(next);
          break;
        } else {
          Expression *pp = (*next)->TrySimplifyMultiplying(*it);
          if (pp) {
            changed = true;
            needContinue = true;
            delete *it;
            delete *next;
            childrenSet.insert(pp);
            childrenSet.erase(it);
            it = childrenSet.erase(next);
            break;
          }
        }
      }
    }
  }
  if (childrenSet.size() == 0) {
    return new Constant(0);
  } else {
    return NULL;
  }
}

Division::Division(Expression *numerator, Expression *denominator) :
    Expression(TypeDivide) {
  this->numerator = numerator;
  this->denominator = denominator;
  if (!numerator || !denominator)
    throw invalid_argument("null argument in Division");
}

bool Division::CanonicalEqualToSameType(Expression *other) {
  assert(other->nodeType() == TypeDivide);
  Division *p = static_cast<Division *>(other);
  return this->denominator->CanonicalEqualTo(p->denominator)
      && this->numerator->CanonicalEqualTo(p->numerator);
}

bool Division::CanonicalSmallerThanSameType(Expression *other) {
  assert(other->nodeType() == TypeDivide);
  Division *p = static_cast<Division *>(other);
  if (this->denominator->CanonicalSmallerThan(p->denominator))
    return true;
  else if (this->numerator->CanonicalSmallerThan(p->denominator))
    return true;
  else
    return false;
}

double Division::operator()(double x) const {
  return (*numerator)(x) / (*denominator)(x);
}

void Division::recursivePrint(string &output, OperatorPrecedence::Order order) const {
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
  if (closeParenthese) output.push_back(')');
}

Expression *Division::diff() const {
  Expression *df_g = new Multiplication(numerator->diff(),
                                        denominator->clone());
  Expression *f_dg = new Multiplication(numerator->clone(),
                                        denominator->diff());
  Expression *g2 = new Multiplication(denominator->clone(),
                                      denominator->clone());
  Expression *dfg_fdg = new Addition(df_g,
                                     new Multiplication(new Constant(-1), f_dg));
  return new Division(dfg_fdg, g2);
}

Expression *Division::clone() const {
  return new Division(numerator->clone(), denominator->clone());
}

Division::~Division() {
  delete numerator;
  delete denominator;
}

Expression *Division::TrySimplifyAdding(Expression *right) {
  // a/c + b/c = (a+b)/c
  if(right->nodeType()==TypeDivide) {
    Division *p = static_cast<Division*>(right);
    if(denominator->CanonicalEqualTo(p->denominator)){
      Expression *a_plus_b= new Addition(numerator->clone(),p->numerator->clone());
      Expression *c = p->denominator->clone();
      Expression *abc= new Division(a_plus_b,c);
      bool changed;
      Expression *simplify=abc->simplify(changed);
      if(simplify) {
        delete abc;
        return simplify;
      }else{
        return abc;
      }
    }
  }
  return NULL;
}

Expression *Division::TrySimplifyMultiplying(Expression *right) {
  if(right->nodeType()==TypeDivide){
    // (a/b)*(c/d)=(a*c)/(b*d)
    Division *p = static_cast<Division*>(right);
    Expression *ac=new Multiplication(numerator->clone(),p->denominator->clone());
    Expression *bd=new Multiplication(denominator->clone(),p->numerator->clone());
    Expression *acbd=new Division(ac,bd);
    bool changed;
    Expression *simplify=acbd->simplify(changed);
    if(simplify) {
      delete acbd;
      return simplify;
    }else{
      return acbd;
    }
  }else{
    // (a/b)*c = a*c / b
    Expression *ac=new Multiplication(numerator->clone(),right->clone());
    Expression *acb=new Division(ac,denominator->clone());
    bool changed;
    Expression *simplify=acb->simplify(changed);
    if(simplify) {
      delete acb;
      return simplify;
    }else{
      return acb;
    }
  }
  return NULL;
}

Expression *Division::simplify(bool &changed) {
  bool childChanged;
  Expression *nume = numerator->simplify(childChanged);
  if (nume) {
    changed = true;
    delete numerator;
    numerator = nume;
  } else {
    changed = childChanged || changed;
  }
  Expression *deno = denominator->simplify(childChanged);
  if (deno) {
    changed = true;
    delete denominator;
    denominator = deno;
  } else {
    changed = childChanged || changed;
  }
  // 0/a = 0
  if (numerator->nodeType() == TypeConstant) {
    Constant *p = static_cast<Constant *>(numerator);
    if (p->value() == 0) {
      return new Constant(0);
    }
  }
  // a/1 = a
  if(denominator->nodeType()==TypeConstant){
    Constant *p = static_cast<Constant *>(denominator);
    if (p->value() == 1) {
      return numerator->clone();
    }
  }
  // (a/b)/(c/d) = ad/bc
  {
    bool numeDivision = numerator->nodeType() == TypeDivide;
    bool denoDivision = denominator->nodeType() == TypeDivide;
    Expression *a = 0, *b = 0, *c = 0, *d = 0;
    if (numeDivision) {
      changed=true;
      Division *p = static_cast<Division *>(numerator);
      //change ownership
      a = p->numerator;
      b = p->denominator;
      p->numerator = NULL;
      p->denominator = NULL;
      delete p;
      numerator = a;
    }
    if (denoDivision) {
      changed=true;
      Division *p = static_cast<Division *>(denominator);
      //change ownership
      c = p->numerator;
      d = p->denominator;
      p->numerator = NULL;
      p->denominator = NULL;
      delete p;
      denominator = c;
    }
    if (b)
      denominator = new Multiplication(denominator, b);
    if (d)
      numerator = new Multiplication(numerator, d);
  }
  return NULL;
}

Composition::Composition(Expression *left, Expression *right) :
    Expression(TypeCompo), left(left), right(right) {
  if (!left || !right) throw invalid_argument("null pointer in Composition");
}

bool Composition::CanonicalEqualToSameType(Expression *other) {
  assert(other->nodeType() == TypeCompo);
  Composition *p = static_cast<Composition *>(other);
  return this->left->CanonicalEqualTo(p->left)
      && this->right->CanonicalEqualTo(p->right);
}

bool Composition::CanonicalSmallerThanSameType(Expression *other) {
  assert(other->nodeType() == TypeCompo);
  Composition *p = static_cast<Composition *>(other);
  if (this->left->CanonicalSmallerThan(p->left))
    return true;
  else if (this->right->CanonicalSmallerThan(p->right))
    return true;
  else
    return false;
}

void Composition::recursivePrint(string &output, OperatorPrecedence::Order order) const {
  bool closeParentheses = false;
  OperatorPrecedence::Order nextOrder;
  if (order >= OperatorPrecedence::Composition) {
    output.push_back('(');
    closeParentheses = true;
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
  if (closeParentheses) output.push_back(')');
}

Expression *Composition::diff() const {
  Expression *dfog = new Composition(left->diff(), right->clone());
  Expression *dg = right->diff();
  return new Multiplication(dfog, dg);
}

Expression *Composition::clone() const {
  return new Composition(left->clone(), right->clone());
}

Expression *Composition::TrySimplifyAdding(Expression *right) {
  return nullptr;
}

Expression *Composition::TrySimplifyMultiplying(Expression *right) {
  return nullptr;
}

Expression *Composition::simplify(bool &changed) {
  return nullptr;
}

bool Constant::CanonicalEqualToSameType(Expression *other) {
  assert(other->nodeType() == TypeConstant);
  const Constant *p = static_cast<const Constant *>(other);
  return c == p->c;
}

bool Constant::CanonicalSmallerThanSameType(Expression *other) {
  assert(other->nodeType() == TypeConstant);
  const Constant *p = static_cast<const Constant *>(other);
  return c < p->c;
}

void Constant::recursivePrint(string &output, OperatorPrecedence::Order order) const {
  stringstream ss;
  ss << this->c;
  output += ss.str();
}

Expression *Constant::TrySimplifyAdding(Expression *right) {
  switch (right->nodeType()) {
    case TypeConstant: {
      Constant *p = static_cast<Constant *>(right);
      return new Constant(this->c + p->c);
    }
    case TypeVariable: {
      return new Polynomial(1, this->c);
    }
    case TypePoly: {
      Polynomial *p = static_cast<Polynomial * >(right);
      vector<double> para = p->getParameter();
      assert(para.size() > 1);
      para[0] += this->c;
      return new Polynomial(para);
    }
    default:
      return NULL;
  }
}

Expression *Constant::TrySimplifyMultiplying(Expression *right) {
  assert(this->c != 0);
  switch (right->nodeType()) {
    case TypeConstant: {
      Constant *p = static_cast<Constant *>(right);
      return new Constant(this->c * p->c);
    }
    case TypeVariable: {
      return new Polynomial(this->c, 0);
    }
    case TypePoly: {
      Polynomial *p = static_cast<Polynomial * >(right);
      vector<double> para = p->getParameter();
      assert(para.size() > 1);
      for (auto it = para.begin(); it != para.end(); it++) {
        *it *= this->c;
      }
      return new Polynomial(para);
    }
    default:
      return NULL;
  }
}

Expression *VariableX::TrySimplifyAdding(Expression *right) {
  switch (right->nodeType()) {
    case TypeConstant:
      assert(false);//should be treated by Constant
    case TypeVariable: {
      return new Polynomial(2, 0);
    }
    case TypePoly: {
      Polynomial *p = static_cast<Polynomial * >(right);
      vector<double> para = p->getParameter();
      assert(para.size() > 1);
      para[1] += 1;
      return new Polynomial(para);
    }
    default:
      return NULL;
  }
}

Expression *VariableX::TrySimplifyMultiplying(Expression *right) {
  switch (right->nodeType()) {
    case TypeVariable: {
      vector<double> para;
      para.push_back(0);
      para.push_back(0);
      para.push_back(1);
      return new Polynomial(para);
    }
    case TypePoly: {
      Polynomial *p = static_cast<Polynomial * >(right);
      vector<double> para = p->getParameter();
      assert(para.size() > 1);
      para.push_back(0);
      for (auto it = para.rbegin(); it != para.rend(); it++) {
        *it = *(it + 1);
      }
      para[0] = 0;
      return new Polynomial(para);
    }
    default:
      return NULL;
  }
}

Polynomial::Polynomial(const vector<double> &parametre) :
    Expression(TypePoly) {
  this->para = parametre;
  bool isConst = true;
  if (para.size() <= 1) {
    isConst = true;
  } else {
    for (int i = 1; i < para.size(); i++)
      if (para[i] != 0) isConst = false;
  }
  if (isConst)
    throw invalid_argument(
        "Degenerate case not allowed, Polynomial can't be constant");
  // clear zeros at the end of list
  for (int i = para.size() - 1; i >= 0; i--) {
    if (para[i] == 0)
      para.pop_back();
    else
      break;
  }
  assert(para.size() >= 2);
}

Polynomial::Polynomial(double a, double b) :
    Expression(TypePoly) {
  if (a == 0)
    throw invalid_argument(
        "Degenerate case not allowed, Polynomial can't be constant");
  this->para.push_back(b);
  this->para.push_back(a);
}

bool Polynomial::CanonicalEqualToSameType(Expression *other) {
  assert(other->nodeType() == TypePoly);
  Polynomial *p = static_cast<Polynomial *>(other);
  if (para.size() != p->para.size()) {
    return false;
  } else {
    for (int i = para.size() - 1; i >= 0; i--)
      if (para[i] != p->para[i]) return false;
    return true;
  }
}

bool Polynomial::CanonicalSmallerThanSameType(Expression *other) {
  assert(other->nodeType() == TypePoly);
  Polynomial *p = static_cast<Polynomial *>(other);
  if (para.size() < p->para.size()) {
    return true;
  } else {
    for (int i = para.size() - 1; i >= 0; i--)
      if (para[i] < p->para[i]) return true;
    return false;
  }
}

double Polynomial::operator()(double x) const {
  assert(para.size() > 0);
  double xPowerK = 1;
  double sum = 0;
  for (vector<double>::const_iterator it = para.begin(); it != para.end();
       it++) {
    sum += (*it) * xPowerK;
    xPowerK *= x;
  }
  return sum;
}

void Polynomial::recursivePrint(string &output, OperatorPrecedence::Order order) const {
  stringstream ss;
  // special treatment to the first constant
  // not "ax^0", but "a"
  assert(para.size() >= 2);
  if (para[0] != 0) ss << para[0];
  // special treatment to the second term
  // not "ax^1", but "ax"
  if (para[1] != 0) {
    if (para[1] > 0) ss << "+";
    ss << para[1] << "x";
  }

  for (int i = 2; i < para.size(); i++) {
    if (para[i] == 0) continue;
    if (para[i] > 0) ss << "+";
    ss << para[i] << "x^" << i;
  }

  string s("Poly[");
  s += ss.str();
  s += "]";
  output += s;
}

Expression *Polynomial::diff() const {
  assert(para.size() >= 2);
  vector<double> temp;
  vector<double>::const_iterator it = para.begin();
  it++;
  int k = 1;
  for (; it != para.end(); it++) {
    temp.push_back(k * (*it));    // d(a*x^k)=a*k*x^(k-1)
    k++;
  }
  assert(temp.size() >= 0);    // polynome isn't allowed to be constant
  if (temp.size() == 1)
    return new Constant(temp[0]);    // function is affine = ax+b, df=a
  else
    return new Polynomial(temp);
}

Expression *Polynomial::clone() const {
  return new Polynomial(*this);
}

Expression *Polynomial::TrySimplifyAdding(Expression *right) {
  switch (right->nodeType()) {
    case TypeConstant://should be treated by Constant
    case TypeVariable:// etc.
      assert(false);
      break;
    case TypePoly: {
      Polynomial *p = static_cast<Polynomial * >(right);
      assert(p->para.size() > 1);
      int n = max(this->para.size(), p->para.size());
      vector<double> newPara(n, 0);
      for (int i = 0; i < n; i++) {
        newPara[i] = this->para[i] + p->para[i];
      }
      for (int i = newPara.size() - 1; i >= 0; i--) {
        if (newPara[i] == 0)
          newPara.pop_back();
        else
          break;
      }
      if (newPara.size() > 1)
        return new Polynomial(newPara);
      else if (newPara.size() == 1) {
        return new Constant(newPara[0]);
      } else {
        return new Constant(0);
      }
    }
    default:
      return NULL;
  }
}

Expression *Polynomial::TrySimplifyMultiplying(Expression *right) {
  switch (right->nodeType()) {
    case TypeConstant:
    case TypeVariable:
      assert(false);
      break;
    case TypePoly: {
      Polynomial *p = static_cast<Polynomial * >(right);
      int n = this->para.size();
      int m = p->para.size();
      assert(n > 1 && m > 1);
      vector<double> newPara(n + m - 1, 0);
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
          newPara[i + j] += this->para[i] * p->para[j];
        }
      }
      assert(newPara[n + m - 1] != 0);
      return new Polynomial(newPara);
    }
    default:
      return NULL;
  }
}

void ElementryFunction::recursivePrint(string &output, OperatorPrecedence::Order order) const {
  output += functionName();
  output += "(x)";
}

void ElementryFunction::compositionPrint(string &output, const Expression *innerFunction) const {
  string inner;
  innerFunction->recursivePrint(inner, OperatorPrecedence::None);
  output += functionName();
  output += "(";
  output += inner;
  output += ")";
}

bool Trigo::CanonicalEqualToSameType(Expression *other) {
  assert(other->nodeType() == TypeTrigo);
  Trigo *p = static_cast<Trigo *>(other);
  return this->trigoType == p->trigoType;
}

bool Trigo::CanonicalSmallerThanSameType(Expression *other) {
  assert(other->nodeType() == TypeTrigo);
  Trigo *p = static_cast<Trigo *>(other);
  return this->trigoType < p->trigoType;
}

Expression *Trigo::diff() const {
  Expression *r;
  switch (trigoType) {
    case Sin:
      r = new Trigo(Cos);
      break;
    case Cos: {
      Expression *c = new Trigo(Sin);
      r = new Multiplication(new Constant(-1), c);
      break;
    }
    case Tan: {
      // 1/(cos*cos)
      Expression *c1 = new Trigo(Cos);
      Expression *c2 = new Trigo(Cos);
      Expression *m = new Multiplication(c1, c2);
      r = new Division(new Constant(1), m);
      break;
    }
    default:
      assert(false);
  }
  return r;
}

Expression *Trigo::clone() const {
  return new Trigo(*this);
}

double Trigo::operator()(double x) const {
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

string Trigo::functionName() const {
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

Expression *Logarithm::diff() const {
  // 1/x
  return new Division(new Constant(1), new VariableX);
}

Expression *Logarithm::clone() const {
  return new Logarithm;
}

double Logarithm::operator()(double x) const {
  return log(x);
}

string Logarithm::functionName() const {
  return string("ln");
}

Expression *Logarithm::TrySimplifyAdding(Expression *right) {
  //TODO
  return nullptr;
}

Expression *Logarithm::TrySimplifyMultiplying(Expression *right) {
  //TODO
  return nullptr;
}

Expression *Logarithm::simplify(bool &changed) {
  //TODO
  return nullptr;
}

Expression *Exponential::diff() const {
  return new Exponential;
}


Expression *Exponential::clone() const {
  return new Exponential;
}

double Exponential::operator()(double x) const {
  return exp(x);
}

string Exponential::functionName() const {
  return string("exp");
}

bool ExpressionComparator::operator()(Expression *left, Expression *right) {
  return left->CanonicalSmallerThan(right);
}

Expression *Exponential::TrySimplifyAdding(Expression *right) {
  //TODO
  return nullptr;
}

Expression *Exponential::TrySimplifyMultiplying(Expression *right) {
  //TODO
  return nullptr;
}

Expression *Exponential::simplify(bool &changed) {
  //TODO
  return nullptr;
}
