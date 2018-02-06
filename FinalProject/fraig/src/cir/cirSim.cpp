/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include "cirMgr.h"
#include "cirGate.h"
#include "../util/myHashSet.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
  if(!_isSimFlag) FECBroListInit();
  bool isChangedFlag = false;
  size_t failTime = 0;
  size_t count = 0;
  size_t maxFailTime = 10; //need to modify this.
  //cout<<maxFailTime<<endl;
  while(failTime!=maxFailTime && !_FECBroListPtr->empty())
  {
    //myUsage.report(true, false);
    //cout<<endl;
    count++;
    simRnInput();
    sim();
    writeLog(sizeof(size_t)*8);
//    cout<<(size_t)((pow(2,(maxFailTime-count))))<<endl;
    FECBroListUpdate(isChangedFlag);
    if(!isChangedFlag) 
    {
      failTime++;
    }
    else  
    {
      isChangedFlag = false;
    }
    //cout<<"Total #FEC Group = "<<_FECBroListPtr->size()<<endl;
    //cout<<failTime<<endl;
  }
  simSort();
  if(count>0) _isSimFlag = true;
  cout<<count*sizeof(size_t)*8<<" patterns simulated."<<endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  //cout<<endl;
  if(!_isSimFlag) FECBroListInit();
  bool isChangedFlag = false;
  size_t failTime = _size;
  size_t count = 0;
  size_t totCount = 0;
  while(simFileInput(patternFile, count))//if the pool is empty then return false.
  {
    sim();
    writeLog(count);
    FECBroListUpdate(isChangedFlag);
    if(!isChangedFlag) failTime++;
    totCount += count;
    count=0;
    //cout<<"Total #FEC Group = "<<_FECBroListPtr->size()<<endl;
  }
  simSort();
  if(totCount>0) _isSimFlag = true;
  cout<<totCount<<" patterns simulated."<<endl; 
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void
CirMgr::FECBroListInit()
{
  _FECBroListPtr->push_back(_gatePtrList[0]);
  CirGate* gateLittlestDickPtr = _gatePtrList[0];
  for(auto it=_DFSList.begin(); it!=_DFSList.end(); it++)
  {
    if((*it)->isAIG()) 
    {
      gateLittlestDickPtr->makeADickWith(*it, false);
      gateLittlestDickPtr = *it;
    }
  }
  if(!_gatePtrList[0]->_simDick) _FECBroListPtr->pop_back();
}

void
CirMgr::simRnInput()
{
  for(auto it=_iVec.begin(); it!=_iVec.end(); it++)
  {
    //srand(clock());
    _gatePtrList[(*it)]->_simValue = ((size_t)rand()<<(sizeof(int)*8))+(size_t)rand();
  }
}

bool
CirMgr::simFileInput(ifstream& patternFile, size_t& count)
{
  string str;
  size_t buf[_iSize];
  for(size_t i=0; i!=_iSize; i++) buf[i]=0;

  while(patternFile>>str)
  {
    if(str.size()!=_iSize)
    {
      cerr<<"Error: Pattern("<<str<<") length("<<str.size()
          <<") does not match the number of inputs("<<_iSize<<") in a circuit!!"<<endl;
    }

    for(size_t i=0; i!=_iSize; i++)
    {
      switch(str[i])
      {
        case '0': break;
        case '1': buf[i] |= ((size_t)1<<count); break;
        default: 
          cerr<<"Error: Pattern("<<str<<") contains a non-0/1 character(‘"<<str[i]<<"’)."<<endl;
          return false;
      }
    }

    count++;
    if(count == 8*sizeof(size_t)) break;
  }

  if(count == 0) return false;
  else
  {
    for(size_t i=0; i!=_iSize; i++)
    {
      _gatePtrList[_iVec[i]]->_simValue = buf[i];
    }
    return true;
  }
}

void
CirMgr::writeLog(const size_t& count)
{
  if(!_simLog) return;

  for(size_t i=0; i<count; i++)
  {
    for(size_t j=0; j<_iSize; j++)
    {
      (*_simLog)<<((_gatePtrList[_iVec[j]]->_simValue>>i)&1);
    }
    (*_simLog)<<' ';
    for(size_t j=0; j<_oSize; j++)
    {
      (*_simLog)<<((_gatePtrList[_oVec[j]]->_simValue>>i)&1);
    }
    (*_simLog)<<endl;
  }
}

void
CirMgr::sim()
{
  for(auto it=_DFSList.begin(); it!=_DFSList.end(); it++)
  {
    if((*it)->isAIG()||(*it)->isPO()) (*it)->sim();
  }
}

void
CirMgr::FECBroListUpdate(bool& isChangedFlag)
{
  vector<CirGate*>* FECBroListPtr = new vector<CirGate*>;
  SimKey key;
  CirGate* gatePtr;
  CirGate* tmpPtr;
  for(auto it=_FECBroListPtr->begin(); it!=_FECBroListPtr->end(); it++)
  {
    gatePtr = (*it);
    HashSet<SimKey> hashSet(gatePtr->broSize());
    while(gatePtr)
    {
      tmpPtr = gatePtr->_simDick;
      key._cirGateV.setV(gatePtr, false);
      if(hashSet.query(key)) 
      {
        key.getPtr()->makeADickWith(gatePtr, key.isInv());
        key._cirGateV.setV(gatePtr, false);
        hashSet.update(key);
      }
      else
      {
        hashSet.insert(key);
        FECBroListPtr->push_back(key.getPtr());
        key.getPtr()->beABro();
      }
      gatePtr = tmpPtr;
    }
  }
  auto it =FECBroListPtr->begin();
  while(it!=FECBroListPtr->end())
  {
    if(!((*it)->_simDick)) 
    {
      it = FECBroListPtr->erase(it);
      isChangedFlag = true;
    }
    else it++;
  }
  delete _FECBroListPtr;
  _FECBroListPtr = FECBroListPtr;
}

void
CirMgr::simSort()
{  
  for(auto it=_FECBroListPtr->begin(); it!=_FECBroListPtr->end(); it++)
  {
    vector<CirGate*> gatePtrVec;
    CirGate* gatePtr = *it;
    while(gatePtr)
    {
      gatePtrVec.push_back(gatePtr);
      gatePtr = gatePtr->_simDick;
    }

    sort(gatePtrVec.begin(), gatePtrVec.end(), CirGate::pointerComparing);
    
    auto it1 = gatePtrVec.begin();
    auto it2 = it1;
    it2++;

    (*it) = (*it1);
    bool invFlag = (*it1)->_simBroV.isInv();
    (*it1)->_simBroV.setV((*it1), false);
    CirGateV broV = (*it1)->_simBroV;

    while(it2!=gatePtrVec.end())
    {
      (*it1)->_simDick=(*it2);
      (*it2)->_simBroV.setV(broV, ((*it2)->_simBroV.isInv()!=invFlag));
      it1=it2;
      it2++;
    }
    (*it1)->_simDick=NULL;
  }

  sort(_FECBroListPtr->begin(), _FECBroListPtr->end(), CirGate::pointerComparing);
}
