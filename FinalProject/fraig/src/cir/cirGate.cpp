/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

size_t CirGate::_trvlFlag = 0;

/**************************************/
/*   class CirGate member functions   */
/**************************************/

/**************************************/
void
CirGate::reportGate() const
{
   cout<<"================================================================================"<<endl;
   cout<<"= "<<getTypeStr()<<"("<<_id<<")";
   if(!isSymbolEmpty()) cout<<"\""<<getSymbol()<<"\"";
   cout<<", line "<<_lineNo<<endl;
   cout<<"= FECs:";
   if(_simBroV()->_simDick)
   {
      CirGate* gatePtr = _simBroV();
      while(gatePtr)
      {
         cout<<' ';
         if(gatePtr->_simBroV.isInv()) cout<<'!';
         cout<<gatePtr->_id;
         gatePtr = gatePtr->_simDick;
      }
   }
   cout<<endl;
   cout<<"= Value: ";
   for(size_t i=sizeof(size_t)*8; i!=0; i--)
   {
      cout<<((_simValue>>(i-1))&1);
      if((i%8)==1 && i!=1) cout<<'_';
   }
   cout<<endl;
   cout<<"================================================================================"<<endl;
}

/*************************************/
void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   setFlag();
   DFSReportFanin(level);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   setFlag();
   DFSReportFanout(level);
}

void
CirGate::DFSReportFanin(int level, int layer, bool invFlag) const//layer=0, notFlag=false in default.
{
   setVisited();
   printReportFanin(layer, invFlag);

   if(level)
   {
      if(isAIG() || isPO())
      {
         if(!_in1V()->isVisited()) 
            _in1V()->DFSReportFanin(level-1, layer+2, _in1V.isInv());
         else 
            _in1V()->printReportFanin(layer+2, _in1V.isInv(), level-1);
      }
      if(isAIG())
      {
         if(!_in2V()->isVisited()) 
            _in2V()->DFSReportFanin(level-1, layer+2, _in2V.isInv());
         else 
            _in2V()->printReportFanin(layer+2, _in2V.isInv(), level-1);
      }
   }
}

void
CirGate::DFSReportFanout(int level, int layer, bool invFlag) const//layer=0, notFlag=false in default.
{
   setVisited();
   printReportFanout(layer, invFlag);

   if(level && (isAIG() || isPI() || isCONST()))
   {
      for(auto it=_outputVList.begin(); it!=_outputVList.end(); it++)
      {
         if(!(*it)()->isVisited())
            (*it)()->DFSReportFanout(level-1, layer+2, (*it).isInv());
         else 
            (*it)()->printReportFanout(layer+2, (*it).isInv(), level-1);
      }
   }
}

void
CirGate::printReportFanin(int layer, bool invFlag, bool starFlag) const//starFlag = 0 in default.
{
   for(int i=0; i<layer; i++) cout<<" ";
   if(invFlag) cout<<"!";
   cout<<getTypeStr()<<" "<<_id;
   if(starFlag)
   {
      if(isAIG()) cout<<" (*)";
   }
   cout<<endl;
}

void
CirGate::printReportFanout(int layer, bool invFlag, bool starFlag) const//starFlag = 0 in default.
{
   for(int i=0; i<layer; i++) cout<<" ";
   if(invFlag) cout<<"!";
   cout<<getTypeStr()<<" "<<_id;
   if(starFlag)
   {
      if(!_outputVList.empty()) cout<<" (*)";
   }
   cout<<endl;
}

/**************************************/

void
CirGate::printNetList() const
{
   cout<<getTypeStrTidily();
   if(!isCONST()) cout<<" "<<_id;
   if(!_in1V.isNULL())
   {
      cout<<" ";
      if(_in1V()->isFloating()) cout<<"*";
      if(_in1V.isInv()) cout<<"!";
      cout<<_in1V()->_id;
   }
   if(!_in2V.isNULL())
   {
      cout<<" ";
      if(_in2V()->isFloating()) cout<<"*";
      if(_in2V.isInv()) cout<<"!";
      cout<<_in2V()->_id;
   }
   if(!isSymbolEmpty())
   {
      cout<<" ("<<getSymbol()<<")";
   }
   cout<<endl;
}

/****************************************/
void
CirGate::printWriteAIG() const
{
   if(!isAIG()) return;
   cout<<_id*2;

   if(_in1V.isInv()) 
      cout<<" "<<_in1V()->_id*2+1;
   else
      cout<<" "<<_in1V()->_id*2;
   
   if(_in2V.isInv()) 
      cout<<" "<<_in2V()->_id*2+1;
   else
      cout<<" "<<_in2V()->_id*2;

   cout<<endl;
}

//functions of connecting
CirGateV
CirGate::breakConnectingWithIn1()//return the gate broken.
{
      CirGateV tmp = _in1V;
      for(auto it=_in1V()->_outputVList.begin(); it!=_in1V()->_outputVList.end(); it++)
      {
            if((*it)() == this) 
            {
                  _in1V()->_outputVList.erase(it);
                  break;
            }
      }
      _in1V.setNULL();
      return tmp;
}

CirGateV 
CirGate::breakConnectingWithIn2()
{
      CirGateV tmp = _in2V;
      for(auto it=_in2V()->_outputVList.begin(); it!=_in2V()->_outputVList.end(); it++)
      {
            if((*it)() == this) 
            {
                  _in2V()->_outputVList.erase(it);
                  break;
            }
      }
      _in2V.setNULL();
      return tmp;      
}

CirGateV
CirGate::breakConnectingWithOutBegin()//expect that the outputVList cannot be empty!
{
      CirGateV tmp = _outputVList[0];
      _outputVList.erase(_outputVList.begin());
      if(tmp()->_in1V()==this) tmp()->_in1V.setNULL();
      else tmp()->_in2V.setNULL();
      return tmp;
}

//required that one of input of gateV() must be NULL
void
CirGate::connectAsOutEnd(CirGateV gateV, bool invFlag)//invFlag is false in default. 
{
      if(gateV()->_in1V.isNULL()) gateV()->_in1V.setV(this, invFlag!=gateV.isInv());
      else gateV()->_in2V.setV(this, invFlag!=gateV.isInv());

      if(invFlag) gateV.addInv();
      _outputVList.push_back(gateV);
}







