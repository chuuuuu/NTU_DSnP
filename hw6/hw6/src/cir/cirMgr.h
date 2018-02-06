/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr(): _gateArr(NULL) {}
   ~CirMgr();
   /*{
      if(_isRead) 
      {
         cout<<"haha123"<<endl;
         cout<<_gateArr<<endl;
         delete [] (CirGate*)_gateArr;
         cout<<"QQ123"<<endl;
      }
   }*/

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned) const;

   // Member functions about circuit construction
   bool readCircuit(const string&);
   
   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) const;

private:
   CirGate* _gateArr;
   size_t _size;
   size_t _iSize;
   size_t _oSize;
   size_t _aSize;
   vector<int> _iVec;
   vector<int> _oVec;
};

#endif // CIR_MGR_H
