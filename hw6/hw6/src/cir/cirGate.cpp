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

extern CirMgr *cirMgr;

size_t CirGate::_trvlFlag = 0;
size_t CirGate::_netListFlag = 0;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   if(_type == UNDEF_GATE)
   {
      cerr<<"GATE("<<_id<<") not found!!"<<endl;
      return;
   }
   int count=0;
   int tmp = 0;
   cout<<"=================================================="<<endl
       <<"= ";
       count++;
       switch(_type)
       {
            case UNDEF_GATE:
               cout<<"haha";
               break;
            case PI_GATE:
               cout<<"PI("<<_id<<")";
               count += 5;
               tmp = _id;
               while(tmp/10 != 0)
               {
                  tmp = tmp/10;
                  count++;
               }
               if(!_symbol.empty())
               {
                  cout<<"\""<<_symbol<<"\"";
                  count += _symbol.size();
                  count += 2;
               }
               cout<<", line "<<_lineNum;
               count += 8;
               tmp = _lineNum;
               while(tmp/10 != 0)
               {
                  tmp = tmp/10;
                  count++;
               }
               break;
            case PO_GATE:
               cout<<"PO("<<_id<<")";
               count += 5;
               tmp = _id;
               while(tmp/10 != 0)
               {
                  tmp = tmp/10;
                  count++;
               }
               if(!_symbol.empty())
               {
                  cout<<"\""<<_symbol<<"\"";
                  count += _symbol.size();
                  count += 2;
               }
               cout<<", line "<<_lineNum;
               count += 8;
               tmp = _lineNum;
               while(tmp/10 != 0)
               {
                  tmp = tmp/10;
                  count++;
               }
               break;
            case AIG_GATE:
               cout<<"AIG("<<_id<<"), line "<<_lineNum;
               count += 14;
               tmp = _id;
               while(tmp/10 !=0)
               { 
                  tmp = tmp/10;
                  count++;
               }
               tmp = _lineNum;
               while(tmp/10 !=0)
               {
                  tmp = tmp/10;
                  count++;
               }
               break;
            case CONST_GATE:
               cout<<"CONST(0), line 0";
               count += 16;
               break;
            case TOT_GATE:
               cout<<"haha";
               break;
      }
      for(int i=count; i<48; i++)
      {
         cout<<" ";
      }
      cout<<"="<<endl;
       
   cout<<"=================================================="<<endl;

}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   setFlag();
   DFS(FANIN, level);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   setFlag();
   DFS(FANOUT, level);
}

