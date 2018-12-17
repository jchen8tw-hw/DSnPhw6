/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
unsigned CirGate::_globalref = 0;
void CirGate::FaninDFS(int lit,unsigned maxlev,unsigned curlev = 0) const{
   if(curlev > maxlev) return;
   unsigned* c = this->getFanin();
   for(unsigned i = 0;i<curlev;i++){
      cout << "  ";
   }
   cout << ((lit %2 == 0)? "" : "!")<< this->getTypeStr() << " " << lit/2;
   if(this->isGlobalref() && curlev != maxlev && !(this->getType() == PI_GATE || this->getType() == CONST_GATE || this->getType() == UNDEF_GATE)){
      cout << " (*)" << endl;
      return;
   }
   else{
      cout << endl;
   }
   if(curlev != maxlev) this->setRefToGlobalRef();
   if(this->getType() == PO_GATE){
      cirMgr->Circuit.id2Gate[*c/2]->FaninDFS(*c,maxlev,curlev+1);
   }
   else if(this->getType() == AIG_GATE){
      cirMgr->Circuit.id2Gate[c[0]/2]->FaninDFS(c[0],maxlev,curlev+1);
      cirMgr->Circuit.id2Gate[c[1]/2]->FaninDFS(c[1],maxlev,curlev+1);
   }
   else{
      return;
   }
}
void CirGate::FanoutDFS(int lit,unsigned maxlev,unsigned curlev = 0) const{
   if(curlev > maxlev) return;
   
   const vector<unsigned>& v = const_cast<CirGate*>(this)->getFanout();
   for(unsigned i = 0;i<curlev;i++){
      cout << "  ";
   }
   
   cout << ((lit %2 == 0)? "" : "!")<< this->getTypeStr() << " " << lit/2;
   if(this->isGlobalref()&& !v.empty() && curlev != maxlev){
         cout << " (*)" << endl;
         return;
   }
   else{
      cout << endl;
   }
   if(curlev != maxlev)this->setRefToGlobalRef();
   if(v.empty()){
      return;
   }
   else{
      for(size_t i = 0;i<v.size();i++){
         cirMgr->Circuit.id2Gate[v[i]/2]->FanoutDFS(v[i],maxlev,curlev+1);
      }
   }
}
void
CirGate::reportGate(int id) const
{
   cout << "==================================================" << endl;
   stringstream ss;
   ss << "= " << getTypeStr() << "(" << id << ")";
   if(getType() == PI_GATE || getType() == PO_GATE){
      SymbolGate* s = dynamic_cast<SymbolGate*>(const_cast<CirGate*>(this));
      if(s->getSymbol() != 0){
         ss << "\"" << s->getSymbol() << "\"";
      }      
   }
   ss <<", line "  << getLineNo();
   cout  << ss.str() << setw(50 - ss.str().size()) << "=" << endl;
   cout << "==================================================" << endl;
   //cout << " " <<setiosflags(ios::left) << setw(4) << getTypeStr() << resetiosflags(ios::left) << endl;
}

void
CirGate::reportFanin(int level,int id) const
{
   assert (level >= 0);
   setGlobalref();
   FaninDFS(id*2,level);
}

void
CirGate::reportFanout(int level,int id) const
{
   assert (level >= 0);
   setGlobalref();
   FanoutDFS(id*2,level);
}

