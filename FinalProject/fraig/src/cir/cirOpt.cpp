/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
  DFSMarkAll();//since cirp -g might break the mark.
  for(size_t i=1; i<_size+1; i++)//start from i=1 to size(only check AIG)
  {
    if(_gatePtrList[i] && !_gatePtrList[i]->isVisited() && !_gatePtrList[i]->isPI())
      sweepAIG(i);
  }
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  size_t i = 0;
  for(auto it=_DFSList.begin(); it!=_DFSList.end(); it++)//start from i=1 to size(only check AIG)
  {
    i = (*it)->_id;
    if(_gatePtrList[i] && _gatePtrList[i]->isAIG())
    {
      if(_gatePtrList[i]->_in1V()->isCONST())
      {
        if(_gatePtrList[i]->_in1V.isInv())//in1 is const1
        {
          cout<<"Simplifying: ";
          merge(i, _gatePtrList[i]->_in2V);
        }
        else//in1 is const
        {
          cout<<"Simplifying: ";
          merge(i, _gatePtrList[i]->_in1V);
        }
      }
      else if(_gatePtrList[i]->_in2V()->isCONST())
      {
        if(_gatePtrList[i]->_in2V.isInv())//in2 is const1
        {
          cout<<"Simplifying: ";
          merge(i, _gatePtrList[i]->_in1V);
        }
        else//in2 is const
        {
          cout<<"Simplifying: ";
          merge(i, _gatePtrList[i]->_in2V);
        }
      }
      else if(_gatePtrList[i]->_in1V()==_gatePtrList[i]->_in2V())
      {
        if(_gatePtrList[i]->_in1V==_gatePtrList[i]->_in2V)//identical input
        {
          cout<<"Simplifying: ";
          merge(i, _gatePtrList[i]->_in1V);
        }
        else//inverted input
        {
          cout<<"Simplifying: ";
          merge(i, CirGateV(_gatePtrList[0], false));
        }
      } 
    }
  }
  resetDFSList();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::sweepAIG(size_t idx)//the idx passed may be AIG's or UNDEF's idx
{
  //the output must be not visited.
  if(_gatePtrList[idx]->isAIG())
  {
    cout<<"Sweeping: AIG("<<idx<<") removed..."<<endl;

    if(!_gatePtrList[idx]->_in1V.isNULL())
    {
      _gatePtrList[idx]->breakConnectingWithIn1();
    }
    if(!_gatePtrList[idx]->_in2V.isNULL())
    {  
      _gatePtrList[idx]->breakConnectingWithIn2();
    }

    _aSize--;
  }
  else//must be UNDEF here
  {
    cout<<"Sweeping: UNDEF("<<idx<<") removed..."<<endl;    
  }

  while(!_gatePtrList[idx]->_outputVList.empty())
  {
    _gatePtrList[idx]->breakConnectingWithOutBegin();
  }

  delete _gatePtrList[idx];
  _gatePtrList[idx]=0;
}

void
CirMgr::merge(size_t idx, CirGateV gateV)
{
  cout<<gateV()->_id<<" merging ";
  if(gateV.isInv()) cout<<"!";
  cout<<idx<<"..."<<endl;
  _gatePtrList[idx]->breakConnectingWithIn1();
  _gatePtrList[idx]->breakConnectingWithIn2();
  while(!_gatePtrList[idx]->_outputVList.empty())
  {
    gateV()->connectAsOutEnd(_gatePtrList[idx]->breakConnectingWithOutBegin(), gateV.isInv());
  }
  delete _gatePtrList[idx]; 
  _gatePtrList[idx]=0;
  _aSize--;
}