/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

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
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   ifstream inf(fileName);
   if(!inf.is_open())
   {
      cerr<<"cannot open design \""<<fileName<<"\"!!"<<endl;
      return false;
   }

   readHead(inf);//build gatePtrList and Const Gate here.
   readPI(inf);
   readPO(inf);
   readAIG(inf);
   readSymbol(inf);
   
   buildConnected();
   resetDFSList();

   return true;
}

void
CirMgr::readHead(ifstream& inf)
{
   string str;
   inf>>str;

   inf>>_size;
   inf>>_iSize;
   inf>>_oSize;
   inf>>_oSize;
   inf>>_aSize;

   _gatePtrList = new CirGate*[_size+_oSize+1];
   for(size_t i=0; i<_size+_oSize+1; i++) _gatePtrList[i]=0;
   _gatePtrList[0] = new CirCONSTGate();
}

void
CirMgr::readPI(ifstream& inf)
{
   for(size_t i=0; i<_iSize; i++)
   {
      int idx;
      inf>>idx;
      idx/=2;
      _gatePtrList[idx] = new CirPIGate(2+i, idx);
      _iVec.push_back(idx);
   }
}

void
CirMgr::readPO(ifstream& inf)
{
   for(size_t i=0; i<_oSize; i++)
   {
      int iltr;
      inf>>iltr;
      _gatePtrList[_size+1+i] = new CirPOGate(_iSize+2+i, _size+1+i, iltr);
      _oVec.push_back(_size+1+i);
   }
}

void
CirMgr::readAIG(ifstream& inf)
{
   for(size_t i=0; i<_aSize; i++)
   {
      int id, iLtr1, iLtr2;
      inf>>id;
      id/=2;
      inf>>iLtr1;
      inf>>iLtr2;
      _gatePtrList[id] = new CirAIGGate(_iSize+_oSize+2+i, id, iLtr1, iLtr2);
   }
}

void
CirMgr::readSymbol(ifstream& inf)
{
   char type;
   size_t order;
   string symbol;
   while(true)
   {
      if(!(inf>>type)) break;
      if(type != 'i' && type != 'o') break;
      if(!(inf>>order)) break;

      inf>>symbol;
      
      if(type == 'i')
      {
         _gatePtrList[_iVec[order]]->setSymbol(symbol);
      }
      else//type == 'o'
      {
         _gatePtrList[_oVec[order]]->setSymbol(symbol);
      }
   }
}

void
CirMgr::buildConnected()
{
   size_t idx;
   bool invFlag;

   for(size_t i=0; i<_size+_oSize+1; i++)
   {
      if(_gatePtrList[i])
      {
         if(_gatePtrList[i]->isAIG() || _gatePtrList[i]->isPO())
         {
            idx = _gatePtrList[i]->_in1V.getV()/2;
            invFlag = !(idx*2 == _gatePtrList[i]->_in1V.getV());

            if(!_gatePtrList[idx]) _gatePtrList[idx] = new CirUNDEFGate(idx);

            _gatePtrList[i]->_in1V.setV(_gatePtrList[idx], invFlag);
            _gatePtrList[idx]->_outputVList.push_back(CirGateV(_gatePtrList[i], invFlag));
         }
         if(_gatePtrList[i]->isAIG())
         {
            idx = _gatePtrList[i]->_in2V.getV()/2;
            invFlag = !(idx*2 == _gatePtrList[i]->_in2V.getV());

            if(!_gatePtrList[idx]) _gatePtrList[idx] = new CirUNDEFGate(idx);
            
            _gatePtrList[i]->_in2V.setV(_gatePtrList[idx], invFlag);
            _gatePtrList[idx]->_outputVList.push_back(CirGateV(_gatePtrList[i], invFlag));
         }
      }
   }
}

/**********************************************************/
//reset the DFSList
void
CirMgr::resetDFSList()
{
   _DFSList.clear();
   CirGate::setFlag();

   for(size_t i=_size+1; i<_size+_oSize+1; i++)
   {
      DFSReset(i);
   }
}

void
CirMgr::DFSReset(size_t idx)
{
   _gatePtrList[idx]->setVisited();

   if(!_gatePtrList[idx]->_in1V.isNULL() && !_gatePtrList[idx]->_in1V()->isVisited())
   {
     if(!_gatePtrList[idx]->_in1V()->isUNDEF())
        DFSReset(_gatePtrList[idx]->_in1V()->_id);
     else 
        _gatePtrList[idx]->_in1V()->setVisited();
   }


   if(!_gatePtrList[idx]->_in2V.isNULL() && !_gatePtrList[idx]->_in2V()->isVisited())
   {  
      if(!_gatePtrList[idx]->_in2V()->isUNDEF()) 
        DFSReset(_gatePtrList[idx]->_in2V()->_id);
      else
        _gatePtrList[idx]->_in2V()->setVisited();
   }
   _DFSList.push_back(_gatePtrList[idx]);
}

void 
CirMgr::DFSMarkAll()
{
  CirGate::setFlag();

  for(size_t i=_size+1; i<_size+_oSize+1; i++)
  {
      DFSMark(i);
  }
}

void
CirMgr::DFSMark(size_t idx)
{
   _gatePtrList[idx]->setVisited();

   if(!_gatePtrList[idx]->_in1V.isNULL() && !_gatePtrList[idx]->_in1V()->isVisited())
   {
     if(!_gatePtrList[idx]->_in1V()->isUNDEF())
        DFSMark(_gatePtrList[idx]->_in1V()->_id);
     else 
        _gatePtrList[idx]->_in1V()->setVisited();
   }


   if(!_gatePtrList[idx]->_in2V.isNULL() && !_gatePtrList[idx]->_in2V()->isVisited())
   {  
      if(!_gatePtrList[idx]->_in2V()->isUNDEF()) 
        DFSMark(_gatePtrList[idx]->_in2V()->_id);
      else
        _gatePtrList[idx]->_in2V()->setVisited();
   }
}


/*********************************************************/
//acess function

CirGate* 
CirMgr::getGate(unsigned gid) const
{
   if(_size+_oSize<gid) return 0;
   if(!_gatePtrList[gid]) return 0;
   if(_gatePtrList[gid]->isUNDEF()) return 0;
   return _gatePtrList[gid];
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
void
CirMgr::printSummary() const
{
   cout<<endl
       <<"Circuit Statistics"<<endl
       <<"=================="<<endl
       <<"  PI"<<right<<setw(12)<<_iSize<<endl
       <<"  PO"<<right<<setw(12)<<_oSize<<endl
       <<"  AIG"<<right<<setw(11)<<_aSize<<endl
       <<"------------------"<<endl
       <<"  Total"<<right<<setw(9)<<_iSize+_oSize+_aSize<<endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for (unsigned i = 0, n = _DFSList.size(); i < n; ++i) 
   {
      cout << "[" << i << "] ";
      _DFSList[i]->printNetList();
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(size_t i=0; i<_iSize; i++)
   {
      cout<<" "<<_gatePtrList[_iVec[i]]->_id;
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(size_t i=_size+1; i<=_size+_oSize; i++)
   {
      cout<<" "<<_gatePtrList[i]->_id;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   bool floatingFlag = false;
   bool notUsedFlag = false;
   
   for(size_t i=0; i<_size+_oSize+1; i++)
   {
      if(_gatePtrList[i] && _gatePtrList[i]->isWithFloating())
      {
         if(!floatingFlag)
         {
            cout<<"Gates with floating fanin(s):";
            floatingFlag = true;
         }
         cout<<" "<<_gatePtrList[i]->_id;
      }
   }
   if(floatingFlag) cout<<endl;

   for(size_t i=0; i<_size+1; i++)
   {
      if(_gatePtrList[i] && _gatePtrList[i]->isDefButNotUsed())
      {
         if(!notUsedFlag)
         {
            cout<<"Gates defined but not used  :";
            notUsedFlag = true;
         }
         cout<<" "<<_gatePtrList[i]->_id;
      }
   }
   if(notUsedFlag) cout<<endl;
}

void
CirMgr::printFECPairs() const
{
  if(!_isSimFlag) return;
  size_t i = 0;
  CirGate* gatePtr;
  for(auto it=_FECBroListPtr->begin(); it!=_FECBroListPtr->end(); it++)
  {
    gatePtr = *it;
    cout<<'['<<i<<']';
    while(gatePtr)
    {
      cout<<' ';
      if(gatePtr->_simBroV.isInv()) cout<<'!';
      cout<<gatePtr->_id;
      gatePtr = gatePtr->_simDick;
    }
    cout<<endl;
    i++;
  }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   int aSize=0;
   for(auto it=_DFSList.begin(); it!=_DFSList.end(); it++) if((*it)->isAIG()) aSize++;
   outfile<<"aag "<<_size<<" "<<_iSize<<" 0 "<<_oSize<<" "<<aSize<<endl;
   for(size_t i=0; i<_iSize; i++)
   {
      outfile<<_iVec[i]*2<<endl;
   }
   for(size_t i=0; i<_oSize; i++)
   {
      if(!_gatePtrList[_oVec[i]]->_in1V.isNULL())
      {
        if(_gatePtrList[_oVec[i]]->_in1V.isInv())
          outfile<<_gatePtrList[_oVec[i]]->_in1V()->_id*2+1<<endl;
        else
          outfile<<_gatePtrList[_oVec[i]]->_in1V()->_id*2<<endl;
      }
   }
   for(auto it=_DFSList.begin(); it!=_DFSList.end(); it++)
   {
      (*it)->printWriteAIG();
   }
   for(size_t i=0; i<_iSize; i++)
   {
      if(!_gatePtrList[_iVec[i]]->isSymbolEmpty())
         cout<<"i"<<i<<" "<<_gatePtrList[_iVec[i]]->getSymbol()<<endl;
   }
   for(size_t i=0; i<_oSize; i++)
   {
      if(!_gatePtrList[_oVec[i]]->isSymbolEmpty())
         cout<<"o"<<i<<" "<<_gatePtrList[_oVec[i]]->getSymbol()<<endl;
   }

   cout<<"c"<<endl
       <<"AAG output by Chung-Yang (Ric) Huang"<<endl;
}





void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{/*
   size_t size;
   size_t iSize;
   size_t oSize;
   size_t aSize;
   
   vector<CirGate*> DFSList;

   while(g)
   {
     if(g->_in1V.isNULL() && (g->isAIG() || g->PI() || g->PO()))
   }

   outfile<<"aag "<<_size<<" "<<_iSize<<" 0 "<<_oSize<<" "<<aSize<<endl;
   for(size_t i=0; i<_iSize; i++)
   {
      outfile<<_iVec[i]*2<<endl;
   }
   for(size_t i=0; i<_oSize; i++)
   {
      if(!_gatePtrList[_oVec[i]]->_in1V.isNULL())
      {
        if(_gatePtrList[_oVec[i]]->_in1V.isInv())
          outfile<<_gatePtrList[_oVec[i]]->_in1V()->_id*2+1<<endl;
        else
          outfile<<_gatePtrList[_oVec[i]]->_in1V()->_id*2<<endl;
      }
   }
   for(auto it=_DFSList.begin(); it!=_DFSList.end(); it++)
   {
      (*it)->printWriteAIG();
   }
   for(size_t i=0; i<_iSize; i++)
   {
      if(!_gatePtrList[_iVec[i]]->isSymbolEmpty())
         cout<<"i"<<i<<" "<<_gatePtrList[_iVec[i]]->getSymbol()<<endl;
   }
   for(size_t i=0; i<_oSize; i++)
   {
      if(!_gatePtrList[_oVec[i]]->isSymbolEmpty())
         cout<<"o"<<i<<" "<<_gatePtrList[_oVec[i]]->getSymbol()<<endl;
   }

   cout<<"c"<<endl
       <<"AAG output by Chung-Yang (Ric) Huang"<<endl;
*/  
}
