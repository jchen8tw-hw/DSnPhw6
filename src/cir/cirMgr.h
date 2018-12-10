/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include<unordered_map>

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr {
   public:
    CirMgr() {}
    ~CirMgr() {}

    // Access functions
    // return '0' if "gid" corresponds to an undefined gate.
    CirGate *getGate(unsigned gid) const { return 0; }

    // Member functions about circuit construction
    bool readCircuit(const string &);

    // Member functions about circuit reporting
    void printSummary() const;
    void printNetlist() const;
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void writeAag(ostream &) const;

   private:
    class ParsedCir {
       public:
        ParsedCir() : maxid(0),inputs(0), outputs(0), ands(0) {}
        size_t inputs, outputs, ands,maxid,latches;
        unordered_map<int,CirGate*> id2Gate;
        vector<int> OutGates;
        vector<int> InGates;
        vector<int> AndGates;
    };
    ParsedCir Circuit;
    bool ParseHeader(ifstream&);
    bool GenGates(ifstream&);
    bool ConstructCir(ifstream&);
};

#endif  // CIR_MGR_H
