/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "cirMgr.h"
#include <ctype.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include "cirGate.h"
#include "util.h"

using namespace std;
typedef stringstream ss;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr *cirMgr = 0;

enum CirParseError {
    EXTRA_SPACE,
    MISSING_SPACE,
    ILLEGAL_WSPACE,
    ILLEGAL_NUM,
    ILLEGAL_IDENTIFIER,
    ILLEGAL_SYMBOL_TYPE,
    ILLEGAL_SYMBOL_NAME,
    MISSING_NUM,
    MISSING_IDENTIFIER,
    MISSING_NEWLINE,
    MISSING_DEF,
    CANNOT_INVERTED,
    MAX_LIT_ID,
    REDEF_GATE,
    REDEF_SYMBOLIC_NAME,
    REDEF_CONST,
    NUM_TOO_SMALL,
    NUM_TOO_BIG,

    DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo = 0;   // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool parseError(CirParseError err) {
    switch (err) {
        case EXTRA_SPACE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Extra space character is detected!!" << endl;
            break;
        case MISSING_SPACE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Missing space character!!" << endl;
            break;
        case ILLEGAL_WSPACE:  // for non-space white space character
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Illegal white space char(" << errInt << ") is detected!!"
                 << endl;
            break;
        case ILLEGAL_NUM:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Illegal " << errMsg
                 << "!!" << endl;
            break;
        case ILLEGAL_IDENTIFIER:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Illegal identifier \""
                 << errMsg << "\"!!" << endl;
            break;
        case ILLEGAL_SYMBOL_TYPE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Illegal symbol type (" << errMsg << ")!!" << endl;
            break;
        case ILLEGAL_SYMBOL_NAME:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Symbolic name contains un-printable char(" << errInt
                 << ")!!" << endl;
            break;
        case MISSING_NUM:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Missing " << errMsg << "!!" << endl;
            break;
        case MISSING_IDENTIFIER:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Missing \"" << errMsg
                 << "\"!!" << endl;
            break;
        case MISSING_NEWLINE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": A new line is expected here!!" << endl;
            break;
        case MISSING_DEF:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Missing " << errMsg
                 << " definition!!" << endl;
            break;
        case CANNOT_INVERTED:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": " << errMsg << " " << errInt << "(" << errInt / 2
                 << ") cannot be inverted!!" << endl;
            break;
        case MAX_LIT_ID:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
                 << endl;
            break;
        case REDEF_GATE:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Literal \"" << errInt
                 << "\" is redefined, previously defined as "
                 << errGate->getTypeStr() << " in line " << errGate->getLineNo()
                 << "!!" << endl;
            break;
        case REDEF_SYMBOLIC_NAME:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Symbolic name for \""
                 << errMsg << errInt << "\" is redefined!!" << endl;
            break;
        case REDEF_CONST:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Cannot redefine constant (" << errInt << ")!!" << endl;
            break;
        case NUM_TOO_SMALL:
            cerr << "[ERROR] Line " << lineNo + 1 << ": " << errMsg
                 << " is too small (" << errInt << ")!!" << endl;
            break;
        case NUM_TOO_BIG:
            cerr << "[ERROR] Line " << lineNo + 1 << ": " << errMsg
                 << " is too big (" << errInt << ")!!" << endl;
            break;
        default:
            break;
    }
    return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool CirMgr::ParseHeader(ifstream &aagf) {
    smatch tok;
    regex aagheader("aag ([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)");
    string header;
    if (!getline(aagf, header)) {
        // need to remove later
        cerr << "read failed" << endl;
        return false;
    }
    if (!regex_match(header, tok, aagheader)) {
        // need to remove later
        cerr << "not match" << endl;
        return false;
    }
    ss tmp;
    for (size_t i = 0; i < 5 ; i++) {
        //cerr << tok[i].str() << endl;
        tmp << tok[i + 1].str() << " ";
    }
    tmp >> Circuit.maxid;
    tmp >> Circuit.inputs;
    tmp >> Circuit.latches;
    tmp >> Circuit.outputs;
    tmp >> Circuit.ands;
    //one for const 0
    Circuit.id2Gate = new CirGate *[Circuit.maxid + Circuit.outputs + 1]();
    return true;
}
bool CirMgr::GenGates(ifstream &aagf) {
    regex gateformat("([0-9]+)");
    smatch tok;
    string gate;
    int lit = 0;
    // Gen constant 0
    Circuit.id2Gate[0] = new ConstGate;
    // input
    for (size_t i = 0; i < Circuit.inputs; i++) {
        if (!getline(aagf, gate)) return false;
        if (!regex_match(gate, tok, gateformat)) return false;
        myStr2Int(tok[1], lit);
        Circuit.id2Gate[lit / 2] = new InputGate;
        Circuit.id2Gate[lit / 2]->setLineNo(i + 2);
    }
    // outputs
    for (size_t i = 0; i < Circuit.outputs; i++) {
        if (!getline(aagf, gate)) return false;
        if (!regex_match(gate, tok, gateformat)) return false;
        myStr2Int(tok[1], lit);
        Circuit.id2Gate[Circuit.maxid + 1 + i] = new OutputGate(lit);
        Circuit.id2Gate[Circuit.maxid + 1 + i]->setLineNo(i + 2 +
                                                          Circuit.inputs);
    }
    // AIG
    regex Andformat("([0-9]+) ([0-9]+) ([0-9]+)");
    for (size_t i = 0; i < Circuit.ands; i++) {
        // haven't add symbol parsing
        // if (gate == "c") break;
        if (!getline(aagf, gate)) return false;
        if (!regex_match(gate, tok, Andformat)) return false;
        int lit[3];
        for (size_t j = 0; j < 3; j++) {
            myStr2Int(tok[j + 1].str(), lit[j]);
        }
        Circuit.id2Gate[lit[0] / 2] = new AndGate(lit[1], lit[2]);
        Circuit.id2Gate[lit[0] / 2]->setLineNo(i + 2 + Circuit.outputs +
                                               Circuit.inputs);
    }
    return true;
}
bool CirMgr::ConstructCir() {
    for (size_t i = 0; i < Circuit.maxid + Circuit.outputs + 1; i++) {
        if (Circuit.id2Gate[i]->getType() == AIG_GATE) {
            unsigned *f = Circuit.id2Gate[i]->getFanin();
            // push literal in
            if (Circuit.id2Gate[f[0] / 2] == 0)
                Circuit.id2Gate[f[0] / 2] = new UndefGate;
            Circuit.id2Gate[f[0] / 2]->_fanout.push_back(i * 2 + (f[0] % 2));
            if (Circuit.id2Gate[f[1] / 2] == 0)
                Circuit.id2Gate[f[1] / 2] = new UndefGate;
            Circuit.id2Gate[f[1] / 2]->_fanout.push_back(i * 2 + (f[1] % 2));
        }
        if (Circuit.id2Gate[i]->getType() == PO_GATE) {
            unsigned *f = Circuit.id2Gate[i]->getFanin();
            if (Circuit.id2Gate[*f / 2] == 0)
                Circuit.id2Gate[*f / 2] = new UndefGate;
            Circuit.id2Gate[*f / 2]->_fanout.push_back(i * 2 + (*f % 2));
        }
    }
    return true;
}
bool CirMgr::readCircuit(const string &fileName) {
    ifstream aagf(fileName);
    // finished read in the buf
    if (!ParseHeader(aagf)) return false;
    if (!GenGates(aagf)) return false;

    aagf.close();
    if (!ConstructCir()) return false;

    return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void CirMgr::printSummary() const {
    cout << "Circuit Statistics" << endl
         << "==================" << endl
         << setw(4) << "PI" << setw(12) << Circuit.inputs << endl
         << setw(4) << "PO" << setw(12) << Circuit.outputs << endl
         << setw(5) << "AIG" << setw(11) << Circuit.ands << endl
         << "------------------" << endl
         << setw(7) << "Total" << setw(9)
         << Circuit.inputs + Circuit.outputs + Circuit.ands << endl;
    return;
}
void CirMgr::printNetlistformat(unsigned id,unsigned prid) const {
    cout << "[" << prid << "] " << setiosflags(ios::left) << setw(4) 
            << Circuit.id2Gate[id]->getTypeStr() << resetiosflags(ios::left);
    return;
}
void CirMgr::DFSTravPO(unsigned id,unsigned& prid) const {
    unsigned *c = Circuit.id2Gate[id]->getFanin();
    //todo: print symbol
    if (Circuit.id2Gate[id]->getType() == PO_GATE) {
        if (!Circuit.id2Gate[*c/2]->isGlobalref()) DFSTravPO(*c/2,prid);
        printNetlistformat(id,prid);
        cout << id << " " << ((*c % 2 == 1) ? "!" : "") << *c/2 << " " << endl;
        
    } else if (Circuit.id2Gate[id]->getType() == PI_GATE) {
        printNetlistformat(id,prid);
        cout << id << " " << endl;
    } else if (Circuit.id2Gate[id]->getType() == AIG_GATE) {
        if (!Circuit.id2Gate[c[0]/2]->isGlobalref()) {
            DFSTravPO(c[0]/2,prid);
        }
        if (!Circuit.id2Gate[c[1]/2]->isGlobalref()) {
            DFSTravPO(c[1]/2,prid);
        }
        printNetlistformat(id,prid);
        cout << id << " " << ((c[0] % 2 == 1) ? "!" : "") << c[0]/2 << " " << ((c[1] % 2 == 1) ? "!" : "") << c[1]/2 << " " << endl;

    } else if (Circuit.id2Gate[id]->getType() == UNDEF_GATE) {
        Circuit.id2Gate[id]->setRefToGlobalRef();
        return;
    }else if (Circuit.id2Gate[id]->getType() == CONST_GATE){
        printNetlistformat(id,prid);
        cout << endl;
    } else {
        //gate unknown type
        printNetlistformat(id,prid);
        cout << "0" << endl;
    }
    Circuit.id2Gate[id]->setRefToGlobalRef();
    prid++;
    return;
}

void CirMgr::printNetlist() const {
    CirGate::setGlobalref();
    static unsigned printid = 0;
    printid = 0;
    //for( size_t i = 0;i<(Circuit.maxid + Circuit.outputs + 1);i++){
    for (size_t i = Circuit.maxid + 1; i < Circuit.maxid + Circuit.outputs + 1;
         i++) {
        DFSTravPO(i,printid);
    //    cout << "id: "  << i << " " << Circuit.id2Gate[i]->getTypeStr() << endl;
    }
}

void CirMgr::printPIs() const {
    cout << "PIs of the circuit:";
    cout << endl;
}

void CirMgr::printPOs() const {
    cout << "POs of the circuit:";
    cout << endl;
}

void CirMgr::printFloatGates() const {}

void CirMgr::writeAag(ostream &outfile) const {}
