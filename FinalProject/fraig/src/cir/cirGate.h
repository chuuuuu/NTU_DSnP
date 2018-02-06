/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGateV
{
public:
   //constructor and destructor
   CirGateV(): _cirGatePtr(0) {}
   CirGateV(size_t ltr): _cirGatePtr(ltr) {}
   CirGateV (CirGate* cirGatePtr): _cirGatePtr((size_t)(cirGatePtr)) {}
   CirGateV (CirGate* cirGatePtr, bool inv): _cirGatePtr((size_t)(cirGatePtr)) 
   { 
      if(inv) addInv();
   }

   //access function
   inline CirGate* getPtr() const { return (CirGate*)((_cirGatePtr|1)^1); }
   inline size_t getV() const { return _cirGatePtr; }
   inline bool isInv() const { return (_cirGatePtr&1); }
   inline bool isNULL() const { return (_cirGatePtr==0); }

   //set function
   inline void addInv() { _cirGatePtr = (_cirGatePtr^1); }
   inline void setV(CirGate* cirGatePtr, bool inv)
   {
      _cirGatePtr = (size_t)(cirGatePtr);
      if(inv) addInv();
   }
   inline void setV(CirGateV cirGateV, bool inv) 
   {
      _cirGatePtr = cirGateV._cirGatePtr;
      if(inv) addInv();
   }
   inline void setNULL() { _cirGatePtr=0; }
   inline void setInv() { if(!isInv()) addInv(); }
   inline void setNoInv() { _cirGatePtr = ((_cirGatePtr>>1)<<1); }

   //operator overloading
   CirGate* operator () () const { return (CirGate*)((_cirGatePtr|1)^1); }
   void operator = (const CirGate* gatePtr) { _cirGatePtr = (size_t)(gatePtr); }
   void operator = (const CirGateV gateV) { _cirGatePtr = gateV.getV(); }
   bool operator == (const CirGateV gateV) const {return (_cirGatePtr== gateV._cirGatePtr);} 
   bool operator < (const CirGateV gateV) const { return (getPtr()<gateV.getPtr()); }
private:
   size_t _cirGatePtr;
};

class CirGate
{
public:
   friend class CirMgr;
   friend class StrashKey;
   friend class SimKey;
   CirGate(size_t lineNo=0, size_t id=0, size_t ltr1=0, size_t ltr2=0):
   _in1V(CirGateV(ltr1)), _in2V(CirGateV(ltr2)), _simValue(0), _satVariable(0), 
   _simBroV(CirGateV(this)), _simDick(0), _myFlag(0), _lineNo(lineNo), _id(id) {}
   virtual ~CirGate() {}

   // Basic access methods
   virtual size_t getId() { return _id; }
   virtual void setSymbol(string& str) {}
   virtual string getSymbol() const { return ""; }
   virtual string getTypeStr() const = 0;
   virtual string getTypeStrTidily() const = 0;
   size_t getLineNo() const { return _lineNo; }

   //virtual CirGateV& getIn1V() { return _in1V; }
   //virtual CirGateV& getIn2V() { return _in2V; }
   virtual bool isAig() const { return false; }
   virtual bool isAIG() const { return false; }
   virtual bool isUNDEF() const { return false; }
   virtual bool isCONST() const { return false; }
   virtual bool isPO() const { return false; }
   virtual bool isPI() const { return false; }
   virtual bool isDefButNotUsed() const { return _outputVList.empty(); }
   virtual GateType getGateType() const = 0;

   virtual bool isSymbolEmpty() const { return true; }
   virtual bool isFloating() const { return false; }
   virtual bool isWithFloating() const { return false; }

   void connectCirGate(CirGate* cirGate);

   // Printing functions
   virtual void printGate() const {}
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

   void DFSReportFanin(int level, int layer=0, bool invFlag=false) const;
   void DFSReportFanout(int level, int layer=0, bool invFlag=false) const;
   void printReportFanin(int layer, bool invFlag, bool starFlag=0) const;
   void printReportFanout(int layer, bool invFlag, bool starFlag=0) const;
   void printNetList() const;
   void printWriteAIG() const;

   static void setFlag() { _trvlFlag++; }
   void setVisited() const { _myFlag = _trvlFlag; }
   bool isVisited() const { return (_myFlag == _trvlFlag); }

  //function of connecting
   CirGateV breakConnectingWithIn1();//return the gate broken.
   CirGateV breakConnectingWithIn2();
   CirGateV breakConnectingWithOutBegin();
   void connectAsOutEnd(CirGateV gateV, bool invFlag=false);

  //function for sim
   virtual void sim() {};
   void makeADickWith(CirGate* gatePtr, bool invFlag)
   {
     _simDick = gatePtr;
     gatePtr->_simBroV = _simBroV;
     if(invFlag) gatePtr->_simBroV.addInv();
     gatePtr->_simDick = NULL;
   }
   inline void beABro()
   {
     _simDick = NULL;
     _simBroV.setV(this, false);
   }
   inline void breakADick()
   {
     CirGate* gatePtr = _simDick->_simDick;
     _simDick->_simBroV.setV(_simDick, false);
     _simDick->_simDick = NULL;
     _simDick = gatePtr;
   }
   inline bool isNotLonely()
   {
     if(_simDick) return true;
     if(_simBroV()!=this) return true;
     return false;
   }
  void electBro()
  {
    if(_simBroV()==this) return;

    bool invFlag = _simBroV.isInv();
    CirGate* gatePtrTmp = _simBroV();
    while(gatePtrTmp)
    {
      if(gatePtrTmp->_simDick == this)
      {
        CirGate* gatePtrTmpTmp = gatePtrTmp->_simDick->_simDick;
        gatePtrTmp->_simDick->_simDick = gatePtrTmp->_simDick->_simBroV();
        gatePtrTmp->_simDick->_simBroV.setV(gatePtrTmp->_simDick, false);
        gatePtrTmp->_simBroV.setV(this, (gatePtrTmp->_simBroV.isInv()!=invFlag));
        gatePtrTmp->_simDick = gatePtrTmpTmp;
        gatePtrTmp = gatePtrTmpTmp;
      }
      else
      {
        gatePtrTmp->_simBroV.setV(this, (gatePtrTmp->_simBroV.isInv()!=invFlag));
        gatePtrTmp = gatePtrTmp->_simDick;
      }
    }
  }

  size_t broSize()
  {
    size_t i=0;
    CirGate* gatePtr = this;
    while(gatePtr)
    {
      i++;
      gatePtr = gatePtr->_simDick;
    }
    return i;
  }

   inline static bool pointerComparing(CirGate* ptr1, CirGate* ptr2)
   {
     return ((ptr1->getId() > ptr2->getId())? false: true);
   }

   static size_t _trvlFlag;

//protected:
   vector<CirGateV> _outputVList;
   CirGateV _in1V, _in2V;
   size_t _simValue;
   Var _satVariable;
   CirGateV _simBroV;
   CirGate* _simDick;//we don't need to record if a gate is FEC with its dick or IFEC.

private:
   mutable size_t _myFlag;
   size_t _lineNo;
   size_t _id;
};

class CirAIGGate : public CirGate
{
public:
   CirAIGGate(size_t lineNo, size_t id, size_t ltr1, size_t ltr2): CirGate(lineNo, id, ltr1, ltr2) {}
   ~CirAIGGate() {}
   bool isAig() const { return true; }
   string getTypeStr() const { return "AIG"; }
   string getTypeStrTidily() const { return "AIG"; }
   bool isAIG() const { return true; }
   GateType getGateType() const { return AIG_GATE; }
   bool isFloating() const { return (_in1V()->isFloating() && _in2V()->isFloating()); }
   bool isWithFloating() const { return (_in1V()->isFloating() || _in2V()->isFloating()); }
   void sim() { _simValue = 
                (_in1V.isInv()? (~(_in1V()->_simValue)): _in1V()->_simValue) & 
                (_in2V.isInv()? (~(_in2V()->_simValue)): _in2V()->_simValue); }
private: 

};

class CirUNDEFGate : public CirGate
{
public:
   CirUNDEFGate(size_t id): CirGate(0, id) {}
   ~CirUNDEFGate() {}
   string getTypeStr() const { return "UNDEF"; }
   string getTypeStrTidily() const { return "UNDEF"; }
   bool isUNDEF() const { return true; }
   bool isFloating() const { return true; }
   bool isWithFloating() const { return false; }
   GateType getGateType() const { return UNDEF_GATE; }
};

class CirCONSTGate : public CirGate
{
public:
   CirCONSTGate(): CirGate(0, 0) {}
   ~CirCONSTGate() {}
   bool isCONST() const { return true; }
   bool isDefButNotUsed() const { return false; }
   string getTypeStr() const { return "CONST"; }
   string getTypeStrTidily() const { return "CONST0"; }
   GateType getGateType() const { return CONST_GATE; }
private:
};

class CirPIGate : public CirGate
{
public:
   CirPIGate(size_t lineNo, size_t id): CirGate(lineNo, id) {}
   ~CirPIGate() {}
   string getTypeStr() const { return "PI"; }
   string getTypeStrTidily() const { return "PI "; }
   void setSymbol(string& symbol) { _symbol = symbol; }
   string getSymbol() const { return _symbol; }
   bool isPI() const { return true; }
   GateType getGateType() const { return PI_GATE; }
   bool isSymbolEmpty() const { return _symbol.empty(); }
private:
   string _symbol;
};

class CirPOGate : public CirGate
{
public:
   CirPOGate(size_t lineNo, size_t id, size_t ltr): CirGate(lineNo, id, ltr) {}
   ~CirPOGate() {}
   string getTypeStr() const { return "PO"; }
   string getTypeStrTidily() const { return "PO "; }
   void setSymbol(string& symbol) { _symbol = symbol; }
   string getSymbol() const { return _symbol; }
   bool isPO() const { return true; }
   GateType getGateType() const { return PO_GATE; }
   bool isSymbolEmpty() const { return _symbol.empty(); }
   bool isFloating() const { return _in1V.isNULL()? true: (_in1V()->isFloating()); }//maybe other function like this need to simplify.
   bool isWithFloating() const { return _in1V.isNULL()? true: (_in1V()->isFloating()); }
   void sim() { _simValue = (_in1V.isInv()? ~(_in1V()->_simValue): (_in1V()->_simValue)); }
private:
   string _symbol;
};

#endif // CIR_GATE_H
