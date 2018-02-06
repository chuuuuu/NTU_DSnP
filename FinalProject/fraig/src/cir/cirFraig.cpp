/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "../util/myHashSet.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  HashSet<StrashKey> hashSet(_DFSList.size());
  StrashKey key;
  CirGateV gateV;

  for(auto it=_DFSList.begin(); it!=_DFSList.end(); it++)
  {
    if((*it)->isAIG())
    {
      key._cirGatePtr = (*it);
      if(hashSet.query(key))
      {
        gateV.setV(key._cirGatePtr, false);
        cout<<"Strashing: ";
        merge((*it)->_id, gateV);
      }
      else
        hashSet.insert(key);
    }
  }
  resetDFSList();
}

void
CirMgr::fraig()
{
  SatSolver solver;
  solver.initialize();
  genProofModel(solver);
  solveSats(solver);

  fraigMerge();
  _isSimFlag = false;
  resetDFSList();
  _FECBroListPtr->clear();
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

//STEP1:getVar for PI/AIG/UNDEF(?) and record it. 
//STEP2:addAigCNF for all connections.
void
CirMgr::genProofModel(SatSolver& solver)
{
//step1
  for(size_t i=0; i!=_size; i++)//CONST PI AIG UNDEF
  {
    if(_gatePtrList[i]) 
    {
      _gatePtrList[i]->_satVariable = solver.newVar();
    }
  }
//step2
  for(auto it=_DFSList.begin(); it!=_DFSList.end(); it++)
  {
    CirGate* gatePtr = *it;

    if(gatePtr->isAIG())
    {
      solver.addAigCNF(gatePtr->_satVariable, gatePtr->_in1V()->_satVariable, gatePtr->_in1V.isInv(),
                                              gatePtr->_in2V()->_satVariable, gatePtr->_in2V.isInv());
    }
  }
  //myUsage.report(true, true);    
}

void
CirMgr::solveSats(SatSolver& solver)
{
  vector<CirGateV> SatPool;
  CirGate* gatePtr;

  if(_gatePtrList[0]->_simDick)
  {
    gatePtr = _gatePtrList[0];
    while(gatePtr->isNotLonely())
    {
      //gatePtr->electBro();
      if(solveSat(solver, gatePtr))
      {
        //cout<<"Updating by SAT... Total #FEC Group = "<<_FECBroListPtr->size()<<endl;
        CirGate* gatePtrTmp = gatePtr->_simDick;
        quickSim(solver);
        if(gatePtrTmp == gatePtr->_simDick)
        {
          SatPool.push_back(CirGateV(gatePtr->_simDick, gatePtr->_simDick->_simBroV.isInv()));
          gatePtr->breakADick();
        }
      }
      else
      {
        //cout<<"Updating by UNSAT... Total #FEC Group = "<<_FECBroListPtr->size()<<endl;
        CirGate* gatePtrTmp = gatePtr->_simDick;
        CirGateV gateV(gatePtr, gatePtrTmp->_simBroV.isInv());
        gatePtr->breakADick();
        _fraigMergePool.push_back(pair<size_t, CirGateV>(gatePtrTmp->_id, gateV));
      }
    }
    if(!SatPool.empty())
    {
      CirGate* gatePtr1 = SatPool.back()();
      bool invFlag = SatPool.back().isInv();
      SatPool.pop_back();
      while(!SatPool.empty())
      {
        SatPool.back()()->_simDick = gatePtr1->_simDick;
        SatPool.back()()->_simBroV.setV(gatePtr1, SatPool.back().isInv()!=invFlag);
        gatePtr1->_simDick = SatPool.back()();
        SatPool.pop_back();
      }
      if(gatePtr1->_simDick) _FECBroListPtr->push_back(gatePtr1);
    }
  }

  fraigMerge();
  resetDFSList();

  for(auto it=_DFSList.begin(); it!=_DFSList.end(); it++)
  {
    gatePtr = *it;
    
    while(gatePtr->isNotLonely())
    {
      gatePtr->electBro();
      //printFECPairs();
      if(solveSat(solver, gatePtr))
      {
        //cout<<"Updating by SAT... Total #FEC Group = "<<_FECBroListPtr->size()<<endl;
        CirGate* gatePtrTmp = gatePtr->_simDick;
        quickSim(solver);
        if(gatePtrTmp == gatePtr->_simDick)
        {
          SatPool.push_back(CirGateV(gatePtr->_simDick, gatePtr->_simDick->_simBroV.isInv()));
          gatePtr->breakADick();
        }
      }
      else
      {
        //cout<<"Updating by UNSAT... Total #FEC Group = "<<_FECBroListPtr->size()<<endl;
        CirGate* gatePtrTmp = gatePtr->_simDick;
        CirGateV gateV(gatePtr, gatePtrTmp->_simBroV.isInv());
        gatePtr->breakADick();
        _fraigMergePool.push_back(pair<size_t, CirGateV>(gatePtrTmp->_id, gateV));
      }
    }
    if(!SatPool.empty())
    {
      CirGate* gatePtr1 = SatPool.back()();
      bool invFlag = SatPool.back().isInv();
      SatPool.pop_back();
      while(!SatPool.empty())
      {
        SatPool.back()()->_simDick = gatePtr1->_simDick;
        SatPool.back()()->_simBroV.setV(gatePtr1, SatPool.back().isInv()!=invFlag);
        gatePtr1->_simDick = SatPool.back()();
        SatPool.pop_back();
      }
      if(gatePtr->_simDick) _FECBroListPtr->push_back(gatePtr);
    }
  }
}

bool
CirMgr::solveSat(SatSolver& solver, CirGate* gatePtr)
{
  //cerr<<"Proving ("<<gatePtr->_id<<(gatePtr->_simDick->_simBroV.isInv()?", !":", ")
  //                 <<gatePtr->_simDick->_id<<")..."<<endl;

  Var newVar = solver.newVar();
  solver.addXorCNF(newVar, gatePtr->_satVariable, gatePtr->_simDick->_simBroV.isInv(), 
                           gatePtr->_simDick->_satVariable, false);
  solver.assumeRelease();

  if(_gatePtrList[0]->_simDick) solver.assumeProperty(_gatePtrList[0]->_satVariable, false);//set Const0


  solver.assumeProperty(newVar, true);
  
  return solver.assumpSolve();
}

void
CirMgr::quickSim(SatSolver& solver)
{
  bool isChangedFlag=false;
  for(size_t i=0; i!=_iSize; i++)
  {
    _gatePtrList[_iVec[i]]->_simValue = (( ((size_t)rand() << (sizeof(int)*8)) + (size_t)rand()) << 1)
                                        + solver.getValue(_gatePtrList[_iVec[i]]->_satVariable);
  }
  sim();
  FECBroListUpdate(isChangedFlag);//we dont need to sort the result.
}

void 
CirMgr::fraigMerge()
{
  while(!_fraigMergePool.empty())
  {
    cout<<"Fraig: ";
    merge(_fraigMergePool.back().first, _fraigMergePool.back().second);
    _fraigMergePool.pop_back();
  }
}