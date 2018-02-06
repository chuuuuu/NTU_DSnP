/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#define isInverse(t) ((((size_t)(t)&1)==1)? 1:0)
#define toInverse(t) ((CirGate*)((size_t)(t)|1))
#define getPtr(t) (isInverse(t)? (CirGate*)((size_t)(t)-1):(t))

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"

using namespace std;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes

//size_t _trvlFlag = 0;
//size_t _netListFlag = 0;

enum DFSType
{
   FANIN,
   FANOUT
};

//size_t CirGate::_trvlFlag = 0;

class CirGate
{
public:
   friend class CirMgr;
   CirGate() 
   {
      _myFlag = 0;
      _id = 0; 
      _lineNum = 0;
      _type = UNDEF_GATE;
      _symbol="";
   }
   ~CirGate() {}

   // Basic access methods
   string getTypeStr() const { return _symbol; }
   unsigned getLineNo() const { return _lineNum; }

   // Printing functions
   void printGate() const;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   
   static void setFlag() {_trvlFlag++; _netListFlag = 0;}
   
   void setVisited() const
   {
      _myFlag=_trvlFlag;
      /*cout<<"_myFlag= "<<_myFlag<<endl;
      cout<<"_trvlFlag= "<<_trvlFlag<<endl;
      cout<<endl;*/
   }
   bool isVisited() const {return (_myFlag == _trvlFlag)? true: false;}
   
   void printGate(int layer, bool notFlag, bool preFlag = false, DFSType type = FANIN) const
   {
      for(int i=0; i<layer; i++) cout<<" ";
      if(notFlag) cout<<"!";
      switch(_type)
      {
         case UNDEF_GATE:
            cout<<"UNDEF "<<_id;
            break;
         case PI_GATE:
            cout<<"PI "<<_id;
            break;
         case PO_GATE:
            cout<<"PO "<<_id;
            break;
         case AIG_GATE:
            cout<<"AIG "<<_id;
            break;
         case CONST_GATE:
            cout<<"CONST 0";
            break;
         case TOT_GATE:
            cout<<"haha";
            break;
      }
      if(preFlag)
      {
	 if(type == FANIN)
	 {
	    if(!_iNode.empty()) cout<<" (*)";
	 }
	 if(type == FANOUT)
	 {
	    if(!_oNode.empty()) cout<<" (*)";
	 }
      }
      cout<<endl;
   }
 
   void printPrint() const
   {
      if(_type == UNDEF_GATE) return;
      cout<<"["<<_netListFlag<<"] ";
      switch(_type)
      {
         case UNDEF_GATE:
            cout<<"haha"<<endl;
            break;
         case PI_GATE:
            cout<<"PI  "<<_id;
            if(!_symbol.empty())
            {
               cout<<" ("<<_symbol<<")";
            }
            cout<<endl;
            break;
         case PO_GATE:
            cout<<"PO  "<<_id<<" ";
            if(isInverse(_iNode[0])) cout<<"!";
            cout<<getPtr(_iNode[0])->_id;
            if(!_symbol.empty())
            {
               cout<<" ("<<_symbol<<")";
            }
            cout<<endl;
            break;
         case AIG_GATE:
            cout<<"AIG "<<_id<<" ";
            if(getPtr(_iNode[0])->_type == UNDEF_GATE) cout<<"*";
            if(isInverse(_iNode[0])) cout<<"!";
            cout<<getPtr(_iNode[0])->_id<<" ";
            
            if(getPtr(_iNode[1])->_type == UNDEF_GATE) cout<<"*";
            if(isInverse(_iNode[1])) cout<<"!";
            cout<<getPtr(_iNode[1])->_id<<endl;
            
            break;
         case CONST_GATE:
            cout<<"CONST0"<<endl;
            break;
         case TOT_GATE:
            cout<<"haha"<<endl;
            break;
      }
      _netListFlag++;
   }
   
   void printWrite() const
   {
      if(_type!=AIG_GATE) return;
      cout<<_id*2<<" ";
      
      if(isInverse(_iNode[0])) cout<<getPtr(_iNode[0])->_id*2+1<<" ";
      else cout<<getPtr(_iNode[0])->_id*2<<" ";
      
      if(isInverse(_iNode[1])) cout<<getPtr(_iNode[1])->_id*2+1<<endl;
      else cout<<getPtr(_iNode[1])->_id*2<<endl;
   }
   void DFS() const
   {
      setVisited();
      for(auto it = _iNode.begin(); it != _iNode.end(); it++)
      {
         if(!((getPtr(*it))->isVisited()))
         {
            getPtr(*it)->DFS();
         }
      }
      printPrint();
   }

   void DFS(DFSType type, int level, int layer=0, bool notFlag = false) const
   {
      setVisited();
      printGate(layer, notFlag);

      if(level != 0)
      {
         if(type == FANIN)
         {
            for(auto it = _iNode.begin(); it != _iNode.end(); it++)
            {
               if(!((getPtr(*it))->isVisited()))
               {
                  getPtr(*it)->DFS(type, level-1, layer+2, isInverse(*it));
               }
               else getPtr(*it)->printGate(layer+2, isInverse(*it), level-1, type);
            }
         }
         else if(type == FANOUT)
         {
	    for(auto it = _oNode.begin(); it != _oNode.end(); it++)
            {
               if(!((getPtr(*it))->isVisited()))
               {
                  getPtr(*it)->DFS(type, level-1, layer+2, isInverse(*it));
               }
               else getPtr(*it)->printGate(layer+2, isInverse(*it), level-1, type);
            }
         }
      }
   }

   void DFSW() const
   {
      setVisited();
      for(auto it = _iNode.begin(); it != _iNode.end(); it++)
      {
         if(!((getPtr(*it))->isVisited()))
         {
            getPtr(*it)->DFSW();
         }
      }
      printWrite();
   }

   void DFSCount(int& tmp) const
   {
      if(!isVisited() && _type == AIG_GATE) tmp++;
      setVisited();
      for(auto it = _iNode.begin(); it != _iNode.end(); it++)
      {
	      if(!(getPtr(*it))->isVisited())
	      {
	         getPtr(*it)->DFSCount(tmp);
 	      }
      }
   }

   static size_t _trvlFlag;
   static size_t _netListFlag;

private: 
   mutable size_t _myFlag;
   size_t _id;
   size_t _lineNum;
   GateType _type;
   string _symbol;
   vector<CirGate*> _iNode;
   vector<CirGate*> _oNode;
};


#endif // CIR_GATE_H
