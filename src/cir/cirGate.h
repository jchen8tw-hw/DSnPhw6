/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <iostream>
#include <string>
#include <vector>
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate {
   public:
    CirGate() {}
    virtual ~CirGate() {}

    // Basic access methods
    string getTypeStr() const { return ""; }
    unsigned getLineNo() const { return 0; }

    // Printing functions
    virtual void printGate() const = 0;
    void reportGate() const;
    void reportFanin(int level) const;
    void reportFanout(int level) const;

   private:
   protected:
};
class InputGate : public CirGate {
   public:
    void printGate() const {}

   private:
};
class OutputGate : public CirGate {
   public:
    void printGate() const {}

   private:
};
class AndGate : public CirGate {
   public:
    void printGate() const {}

   private:
};

#endif  // CIR_GATE_H
