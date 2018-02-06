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

// TODO: Feel free to define your own classes, variables, or functions.
#include "cirGate.h"
#include "cirDef.h"

extern CirMgr *cirMgr;

class StrashKey
{
public:
  friend class CirMgr;

  StrashKey() {}
  ~StrashKey() {}
  //produce a hashkey
  //size_t operator () () const { return (_cirGatePtr->_in1V.getV()^_cirGatePtr->_in2V.getV()); }
  inline size_t operator () () const { return (_cirGatePtr->_in1V.getV()*_cirGatePtr->_in2V.getV()); }
  //check if the inputs of two cirgate are identical or inverted
  bool operator == (StrashKey key) const
  {
    if (_cirGatePtr->_in1V==key._cirGatePtr->_in1V && _cirGatePtr->_in2V==key._cirGatePtr->_in2V)
      return true;
    else if(_cirGatePtr->_in1V==key._cirGatePtr->_in2V && _cirGatePtr->_in2V==key._cirGatePtr->_in1V)
      return true;
    return false;
  }

private:
  CirGate* _cirGatePtr;
};

class SimKey
{
public:
  friend class CirMgr;

  SimKey() {}
  SimKey(CirGate* gatePtr): _cirGateV(CirGateV(gatePtr)) {}
  ~SimKey() {}
  inline size_t operator () () const 
  {
    if(_cirGateV()->_simValue & 1) return ~(_cirGateV()->_simValue);
    else return _cirGateV()->_simValue; 
  }
  bool operator == (const SimKey& key)
  { 
    if(_cirGateV()->_simValue == key._cirGateV()->_simValue) return true;
    if(_cirGateV()->_simValue == ~(key._cirGateV()->_simValue))
    {
      _cirGateV.addInv();
      return true;
    }
    return false;
  }
  inline CirGate* getPtr() const { return _cirGateV(); }
  inline bool isInv() const { return _cirGateV.isInv(); } 

private:
  CirGateV _cirGateV;//the inv record if a gate is FEC with its dickowner or IFEC.
};

class CirMgr
{
public:
   CirMgr(): _simLog(0), _gatePtrList(0), _isSimFlag(false) { _FECBroListPtr = new vector<CirGate*>; }
   ~CirMgr() 
   {
      if(_gatePtrList)
      {
         for(size_t i=0; i<_size+_oSize+1; i++)
         {
            if(_gatePtrList[i]) delete _gatePtrList[i];
         }
         delete [] _gatePtrList;
         _gatePtrList=0;
      }
      delete _FECBroListPtr;
   } 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const;

   // Member functions about circuit construction
   bool readCircuit(const string&);
   void readHead(ifstream& inf);
   void readPI(ifstream& inf);
   void readPO(ifstream& inf);
   void readAIG(ifstream& inf);
   void readSymbol(ifstream& inf);
   void buildConnected();

   // Member functions about DFS
   void resetDFSList();
   void DFSReset(size_t idx);
   void DFSMarkAll();
   void DFSMark(size_t idx);

   // Member functions about circuit optimization
   void sweep();
   void sweepAIG(size_t idx);
   void optimize();
   void merge(size_t idx, CirGateV gateV);

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   void FECBroListInit();
   void simRnInput();
   bool simFileInput(ifstream& patternFile, size_t& count);
   void writeLog(const size_t& count);
   void sim();
   void FECBroListUpdate(bool& isChangedFlag);
   void simSort();

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();
   void genProofModel(SatSolver& solver);
   void solveSats(SatSolver& solver);
   bool solveSat(SatSolver& solver, CirGate* gatePtr);
   void quickSim(SatSolver& solver);
   //void electBroFromDFS();
   void fraigMerge();


   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

private:
   ofstream* _simLog;
   CirGate** _gatePtrList;
   size_t _size;
   size_t _iSize;
   size_t _oSize;
   size_t _aSize;
   vector<size_t> _iVec;//maybe we can use array for it.//modify
   vector<size_t> _oVec;
   vector<CirGate*>* _FECBroListPtr;
   vector<CirGate*> _DFSList;
   vector<pair<size_t, CirGateV>> _fraigMergePool;
   bool _isSimFlag;
};

#endif // CIR_MGR_H
