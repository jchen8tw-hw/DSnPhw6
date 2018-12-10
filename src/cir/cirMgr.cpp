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
    regex aagheader("aag ([0-9])* ([0-9])* ([0-9])* ([0-9])* ([0-9])*");
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
    for (size_t i = 0; i < 5; i++) {
        tmp << tok[i].str();
    }
    tmp >> Circuit.maxid;
    tmp >> Circuit.inputs;
    tmp >> Circuit.latches;
    tmp >> Circuit.outputs;
    tmp >> Circuit.ands;
    return true;
}
bool CirMgr::GenGates(ifstream &aagf) {
    regex gateformat("([0-9])*");
    smatch tok;
    string gate;
    int lit = 0;
    for (size_t i = 0; i < Circuit.inputs; i++) {
        if (!getline(aagf, gate)) return false;
        if (!regex_match(gate, tok, gateformat)) return false;
        myStr2Int(tok[0].str(), lit);
        Circuit.InGates.push_back(lit);
        CirGate *c = new InputGate;
        Circuit.id2Gate.insert(pair<int, CirGate *>(lit / 2, c));
    }
    for (size_t i = 0; i < Circuit.outputs; i++) {
        if (!getline(aagf, gate)) return false;
        if (!regex_match(gate, tok, gateformat)) return false;   
        myStr2Int(tok[0].str(), lit);
        Circuit.OutGates.push_back(lit);
        CirGate *c = new OutputGate;
        Circuit.id2Gate.insert(pair<int, CirGate *>(Circuit.maxid+i+1, c));
    }
    regex Andformat("([0-9])* ([0-9])* ([0-9])*");
    while(getline(aagf,gate)){
       if(gate == "c") break;
       if(!regex_match(gate,tok,Andformat)) return false;
       int lit1,lit2,lit3;
       myStr2Int(tok[0].str(),lit1);
       myStr2Int(tok[1].str(),lit2);
       myStr2Int(tok[2].str(),lit3);

    }
}
bool CirMgr::readCircuit(const string &fileName) {
    ifstream aagf(fileName);
    // finished read in the buf
    if (!ParseHeader(aagf)) return false;
    if (!GenGates(aagf)) return false;
    aagf.close();
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
void CirMgr::printSummary() const {}

void CirMgr::printNetlist() const {}

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
