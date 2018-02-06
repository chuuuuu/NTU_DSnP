/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   vector<string> tokens;
   lexOptions(option, tokens, 0);
   
   if(tokens.empty())               return CmdExec::errorOption(CMD_OPT_MISSING, "");
   
   int num;
   if(!myStr2Int(tokens[0], num))
   {
      if(myStrNCmp("-Array", tokens[0], 2)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
      if(tokens.size() == 1) return CmdExec::errorOption(CMD_OPT_MISSING, tokens[0]);
      int size;
      if(!myStr2Int(tokens[1], size)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
      if(size<=0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
      if(tokens.size() == 2) return CmdExec::errorOption(CMD_OPT_MISSING, "");
      if(!myStr2Int(tokens[2], num)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
      if(num<=0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
      if(tokens.size() == 3)
      {
         mtest.newArrs(num, size);
         return CMD_EXEC_DONE;
      }
      else return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[3]);
   }
   if(num<=0)                       return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
   if(tokens.size()==1) 
   {
      mtest.newObjs((size_t)num);
      return CMD_EXEC_DONE;
   }
   if(myStrNCmp("-Array", tokens[1], 2))  return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[1]);
   if(tokens.size()==2)             return CmdExec::errorOption(CMD_OPT_MISSING, tokens[1]);
   
   int size;
   if(!myStr2Int(tokens[2], size))  return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
   if(size<=0)                      return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
   if(tokens.size()==3) 
   {
      mtest.newArrs(num, size);
      return CMD_EXEC_DONE;
   }
   else return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[3]);
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   vector<string> tokens;
   lexOptions(option, tokens, 0);

   if(tokens.empty()) return CmdExec::errorOption(CMD_OPT_MISSING, "");
   if(!myStrNCmp("-Index", tokens[0], 2))
   {
      if(tokens.size()==1) return CmdExec::errorOption(CMD_OPT_MISSING, tokens[0]);
      int idx;
      if(!myStr2Int(tokens[1], idx)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
      if(idx<0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
      if(tokens.size()==2)
      {
         if(idx >= (int)(mtest.getObjListSize()))
         {
            cerr<<"Size of object list ("<<mtest.getObjListSize()<<") is <= "<<idx<<"!!"<<endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
         }
         mtest.deleteObj(idx);
         return CMD_EXEC_DONE;
      }
      if(myStrNCmp("-Array", tokens[2], 2)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
      if(tokens.size()==3)
      {
         if(idx >= (int)(mtest.getArrListSize()))
         {
            cerr<<"Size of array list ("<<mtest.getArrListSize()<<") is <= "<<idx<<"!!"<<endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
         }
         mtest.deleteArr(idx);
         return CMD_EXEC_DONE;
      }
      else return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[3]);
   }
   else if(!myStrNCmp("-Random", tokens[0], 2))
   {
      if(tokens.size()==1) return CmdExec::errorOption(CMD_OPT_MISSING, tokens[0]);
      int num;
      if(!myStr2Int(tokens[1], num)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
      if(num<=0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
      if(tokens.size()==2)
      {
         if(mtest.getObjListSize()==0)
         {
            cerr<<"Size of object list is 0!!"<<endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
         }
         while(num!=0)
         {
            mtest.deleteObj(rnGen(mtest.getObjListSize()));
            num = num-1;
         }  
         return CMD_EXEC_DONE;
      }
      if(myStrNCmp("-Array", tokens[2], 2)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
      if(tokens.size()==3)
      {
         if(mtest.getArrListSize()==0)
         {
            cerr<<"Size of array list is 0!!"<<endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
         }
         while(num!=0)
         {
            mtest.deleteArr(rnGen(mtest.getArrListSize()));
            num = num-1;
         }  
         return CMD_EXEC_DONE;
      }
      else return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[3]);
   }
   else if(!myStrNCmp("-Array", tokens[0], 2))
   {
      if(tokens.size()==1) return CmdExec::errorOption(CMD_OPT_MISSING, "");
      if(!myStrNCmp("-Index", tokens[1], 2))
      {
         if(tokens.size()==2) return CmdExec::errorOption(CMD_OPT_MISSING, tokens[1]);
         int idx;
         if(!myStr2Int(tokens[2], idx)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
         if(idx<0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
         if(idx >= (int)mtest.getArrListSize())
         {
            cerr<<"Size of array list ("<<mtest.getArrListSize()<<") is <= "<<idx<<"!!"<<endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
         }
         if(tokens.size()!=3) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[3]);
         mtest.deleteArr(idx);
         return CMD_EXEC_DONE;
      }
      else if(!myStrNCmp("-Random", tokens[1], 2))
      {
         if(tokens.size()==2) return CmdExec::errorOption(CMD_OPT_MISSING, tokens[1]);
         int num;
         if(!myStr2Int(tokens[2], num)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
         if(num<=0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[2]);
         if(tokens.size()==3)
         {
            if(mtest.getArrListSize()==0)
            {
               cerr<<"Size of array list is 0!!"<<endl;
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
            }
            while(num!=0)
            {
               mtest.deleteArr(rnGen(mtest.getArrListSize()));
               num = num-1;
            }
            return CMD_EXEC_DONE;
         }
         else return (CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[3]));
      }
      else return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
   }
   else return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


