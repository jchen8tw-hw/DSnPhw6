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
#include <unordered_map>
#include <vector>

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
    CirGate *getGate(unsigned gid) const { 
        return (gid <= Circuit.maxid+Circuit.outputs)? Circuit.id2Gate[gid] : 0 ;
    }

    // Member functions about circuit construction
    bool readCircuit(const string &);

    // Member functions about circuit reporting
    void printSummary() const;
    void printNetlist() const;
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void writeAag(ostream &) const;
    class ParsedCir {
       public:
        ParsedCir()
            : maxid(0),
              inputs(0),
              outputs(0),
              ands(0),
              id2Gate(0),
              latches(0),
              PI_list(0) {}
        ~ParsedCir() {
            for (size_t i = 0; i < maxid + outputs; i++) {
                if (id2Gate[i] != 0) delete id2Gate[i];
            }
            delete[] id2Gate;
            delete[] PI_list;
        }
        void writeAig(int id,vector<unsigned>& AIGlist) const;
        size_t inputs, outputs, ands, maxid, latches;
        //stored with id
        unsigned *PI_list;
        //index is id
        CirGate **id2Gate;
    };
    ParsedCir Circuit;

   private:
    void DFSTravPO(unsigned, unsigned &) const;
    void printNetlistformat(unsigned, unsigned) const;
    bool ParseHeader(ifstream &);
    void HeaderError(string& errstr);
    bool GenGates(ifstream &);
    bool ConstructCir();
};

#endif  // CIR_MGR_H
