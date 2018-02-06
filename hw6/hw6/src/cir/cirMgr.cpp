/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <algorithm>
#include <iostream>
#include <iomanip>
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
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
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
CirMgr::~CirMgr()
{
   if(!_gateArr)
   {
      delete [] _gateArr;
   }
}

CirGate*
CirMgr::getGate(unsigned gid) const
{
   if(_size+_oSize<gid) return 0;
   if((_gateArr+gid)->_type == UNDEF_GATE) return 0;
   else return (_gateArr+gid);
}

bool
CirMgr::readCircuit(const string& fileName)
{
   ifstream inf(fileName);
   if(!inf.is_open()) 
   {
      cerr<<"cannot open design \""<<fileName<<"\"!!"<<endl;
      return false;
   }
   else
   {
      string str;
      inf>>str;
      if(str!="aag") return false;
      
      inf>>_size;
      inf>>_iSize;
      inf>>_oSize;//latch = 0
      inf>>_oSize;
      inf>>_aSize;
      
      _gateArr = new CirGate[(_size+_oSize+1)];
      //cout<<_gateArr<<endl;
      
      for(size_t i=0; i<_size+_oSize+1; i++)
      {
         _gateArr[i]._id = i;
      }

      _gateArr[0]._type = CONST_GATE;
      
      for(size_t i=0; i<_iSize; i++)
      {
         int ltr, idx;
         inf>>ltr;
         idx = ltr/2;
         _gateArr[idx]._type = PI_GATE;
         _gateArr[idx]._lineNum = 2+i;

         _iVec.push_back(idx);
      }

      for(size_t i=0; i<_oSize; i++)
      {
         int ltr, idx;
         inf>>ltr;
         idx = ltr/2;
         _gateArr[_size+1+i]._type = PO_GATE;
         _gateArr[_size+1+i]._lineNum = _iSize + 2 + i;
         
         _oVec.push_back(_size+1+i);

         if(ltr != idx*2)
         {
            _gateArr[_size+1+i]._iNode.push_back(toInverse(_gateArr+idx));
            _gateArr[idx]._oNode.push_back(toInverse(_gateArr+_size+1+i));
         }
         else
         {
            _gateArr[_size+1+i]._iNode.push_back((_gateArr+idx));
            _gateArr[idx]._oNode.push_back((_gateArr+_size+1+i));
         }
      }

      for(size_t i=0; i<_aSize; i++)
      {
         int ltr, iLtr1, iLtr2;
         int idx, iIdx1, iIdx2;
         
         inf>>ltr;
         idx = ltr/2;

         inf>>iLtr1;
         iIdx1 = iLtr1/2;
         
         inf>>iLtr2;
         iIdx2 = iLtr2/2;

         _gateArr[idx]._type = AIG_GATE;
         _gateArr[idx]._lineNum = _iSize + _oSize + 2 + i;

         if(iIdx1*2 != iLtr1)
         {
            _gateArr[idx]._iNode.push_back(toInverse(_gateArr+iIdx1));
            _gateArr[iIdx1]._oNode.push_back(toInverse(_gateArr+idx));
         }
         else
         {
            _gateArr[idx]._iNode.push_back((_gateArr+iIdx1));
            _gateArr[iIdx1]._oNode.push_back((_gateArr+idx));
         }

         if(iIdx2*2 != iLtr2)
         {
            _gateArr[idx]._iNode.push_back(toInverse(_gateArr+iIdx2));
            _gateArr[iIdx2]._oNode.push_back(toInverse(_gateArr+idx));
         }
         else
         {
            _gateArr[idx]._iNode.push_back((_gateArr+iIdx2));
            _gateArr[iIdx2]._oNode.push_back((_gateArr+idx));
         }
      }//need to record the symbol
      while(true)
      {
         char type;
         size_t lineNum;
         if(!(inf>>type)) break;
         if(type != 'i' && type != 'o') break;
         if(!(inf>>lineNum)) break;
         if(type == 'i')
         {
            inf>>_gateArr[_iVec[lineNum]]._symbol;
            //cout<<iLineToIdx[lineNum]<<": "<<_gateArr[iLineToIdx[lineNum]]._symbol<<endl;
         }
         else if(type == 'o')
         {
            inf>>_gateArr[_oVec[lineNum]]._symbol;
            //cout<<oLineToIdx[lineNum]<<": "<<_gateArr[iLineToIdx[lineNum]]._symbol<<endl;
         }
      }
   }
   for(size_t i=0; i<=_size+_oSize; i++)
      sort(_gateArr[i]._oNode.begin(), _gateArr[i]._oNode.end());
   return true;
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
   cout<<endl;
   cout<<"Circuit Statistics"<<endl
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
   CirGate::setFlag();
   cout<<endl;
   for(size_t i=_size+1; i<=_size+_oSize; i++)
   {
      _gateArr[i].DFS();
   }

}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(size_t i=0; i<_iSize; i++)
   {
      cout<<" "<<_gateArr[_iVec[i]]._id;
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(size_t i=_size+1; i<=_size+_oSize; i++)
   {
      cout<<" "<<_gateArr[i]._id;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   bool floatingFlag=false;
   bool notUseFlag=false;
   for(size_t i=0; i<=_size; i++)
   {
      if(_gateArr[i]._type == AIG_GATE)
      {
         for(auto it = _gateArr[i]._iNode.begin(); it != _gateArr[i]._iNode.end(); it++)
         {
            if(getPtr(*it)->_type == UNDEF_GATE)
            {
               if(!floatingFlag)
               {
                  cout<<"Gates with floating fanin(s):";
                  floatingFlag = true;
               }
               cout<<" "<<_gateArr[i]._id; 
               break;
            }
         }
      }
   }
   if(floatingFlag) cout<<endl;
   
   for(size_t i=0; i<=_size; i++)
   {
      if(_gateArr[i]._type == AIG_GATE || _gateArr[i]._type == PI_GATE)
      {
         if(_gateArr[i]._oNode.empty())
         {
            if(!notUseFlag)
            {
               cout<<"Gates defined but not used  :";
               notUseFlag = true;
            }
            cout<<" "<<_gateArr[i]._id;
         }
      }
   }
   if(notUseFlag) cout<<endl;
}

void
CirMgr::writeAag(ostream& outfile) const
{
   int tmp=0;
   CirGate::setFlag();
   for(size_t i=0; i<_oSize; i++)
   {
      _gateArr[_oVec[i]].DFSCount(tmp);
   }
   outfile<<"aag "<<_size<<" "<<_iSize<<" 0 "<<_oSize<<" "<<tmp<<endl;
   for(size_t i=0; i<_iSize; i++)
   {
      outfile<<_iVec[i]*2<<endl;
   }
   for(size_t i=0; i<_oSize; i++)
   {
      if(isInverse(_gateArr[_oVec[i]]._iNode[0]))
      {
         outfile<<((getPtr(_gateArr[_oVec[i]]._iNode[0])->_id)*2+1)<<endl;
      }
      else
      {
         outfile<<(getPtr(_gateArr[_oVec[i]]._iNode[0])->_id)*2<<endl;
      }
   }
   CirGate::setFlag();
   for(size_t i=0; i<_oSize; i++)
   {
      _gateArr[_oVec[i]].DFSW();
   }
   for(size_t i=0; i<_iSize; i++)
   {
      if(!_gateArr[_iVec[i]]._symbol.empty())
      {
         cout<<"i"<<i<<" "<<_gateArr[_iVec[i]]._symbol<<endl;
      }
   }
   for(size_t i=0; i<_oSize; i++)
   {
      if(!_gateArr[_oVec[i]]._symbol.empty())
      {
         cout<<"o"<<i<<" "<<_gateArr[_oVec[i]]._symbol<<endl;
      }
   }

   cout<<"c"<<endl
       <<"AAG output by Chung-Yang (Ric) Huang"<<endl;
}
