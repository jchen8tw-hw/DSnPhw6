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
// Not yet remembering the lineNo.
class CirGate {
   public:
   friend class CirMgr;
    CirGate() : _ref(0), _lineNo(0),_fanin(0),_symbol(0) {}
    virtual ~CirGate() { delete [] _fanin;}

    // Basic access methods
    // string getTypeStr() const { return ""; }
    virtual string getTypeStr() const = 0;
    virtual GateType getType() const = 0;
    unsigned getLineNo() const { return _lineNo; }
    void setLineNo(unsigned n) { _lineNo = n; }
    unsigned* getFanin() const {return _fanin;}
    unsigned* getFanin() {return _fanin;}
    vector<unsigned>& getFanout() {return _fanout;}
    static void setGlobalref() {_globalref++;}
    bool isGlobalref() const {return (_globalref == _ref);}
    void setRefToGlobalRef() const {_ref = _globalref;}

    
    // Printing functions
    virtual void printGate() const = 0;
    void reportGate() const;
    void reportFanin(int level) const;
    void reportFanout(int level) const;

   private:
   protected:
    // using literal
    vector<unsigned> _fanout;
    unsigned* _fanin;
    unsigned _lineNo;
    char* _symbol;
    mutable unsigned _ref;
    static unsigned _globalref;
};
class InputGate : public CirGate {
   public:
    InputGate() : CirGate() {}
    string getTypeStr() const { return "PI"; }
    GateType getType() const { return PI_GATE; }
    void printGate() const { cout << getTypeStr(); }

   private:
    // using literal
};
class OutputGate : public CirGate {
   public:
    OutputGate(unsigned in) : CirGate() {_fanin = new unsigned(in);}
    string getTypeStr() const { return "PO"; }
    GateType getType() const { return PO_GATE; }
    void printGate() const { cout << getTypeStr(); }

   private:
    // using literal
};
class AndGate : public CirGate {
   public:
    AndGate() : CirGate() {}
    AndGate(unsigned fan1, unsigned fan2) : CirGate() {
        _fanin = new unsigned[2];
        _fanin[0] = fan1;
        _fanin[1] = fan2;
    }
    void printGate() const { cout << getTypeStr(); }
    string getTypeStr() const { return "AIG"; }
    GateType getType() const { return AIG_GATE; }

   private:
    // using literal
};
class ConstGate : public CirGate {
   public:
    ConstGate() : CirGate() {}
    void printGate() const { cout << getTypeStr(); }
    string getTypeStr() const { return "CONST0"; }
    GateType getType() const { return CONST_GATE; }

   private:
    // using literal
};

class UndefGate : public CirGate {
   public:
    UndefGate() : CirGate() {}
    void printGate() const { cout << getTypeStr(); }
    string getTypeStr() const { return "UNDEF"; }
    GateType getType() const { return UNDEF_GATE; }

   private:
};

#endif  // CIR_GATE_H
