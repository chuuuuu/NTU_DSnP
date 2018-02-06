/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "util.h"
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
   // TODO...
   if(_dofileStack.size()==1000) return false;
   ifstream* ifs = new ifstream(dof.c_str());
   if(!ifs->is_open()) return false;
   _dofileStack.push(ifs);
   _dofile = _dofileStack.top();
   return true;
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
   assert(_dofile != 0);
   delete _dofile;
   _dofileStack.pop();
   // TODO...
   if(!_dofileStack.empty())
      _dofile = _dofileStack.top();
   else _dofile = 0;
}

// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);
   }

   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str;
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp);
   assert(e != 0);
   e->setOptCmd(optCmd);

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
   bool newCmd = false;
   if (_dofile != 0)
      newCmd = readCmd(*_dofile);
   else
      newCmd = readCmd(cin);

   // execute the command
   if (newCmd) {
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0)
         return e->exec(option);
   }
   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
   // TODO...
   //map<const string, CmdExec*>::iterator it = _cmdMap.begin();
   for(auto it = _cmdMap.begin(); it != _cmdMap.end(); it++)
   {
      CmdExec* e = it->second;
      e -> help();
   }
   cout<<endl;
}

void
CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
CmdExec*
CmdParser::parseCmd(string& option)
{
   // TODO...
   assert(_tempCmdStored == false);
   assert(!_history.empty());
   option = _history.back();
   string token;
   size_t n;
   n = myStrGetTok(option, token);
   if(n!=string::npos) option=option.substr(n);
   else option="";
   //assert(option[0] != 0 && option[0] != ' ');
   CmdExec* e = getCmd(token);
   if(e == 0) cout<<"Illegal command!! ("<<token<<")"<<endl;
   return e;
}

// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//    --- 1.1 ---
//    [Before] Null cmd
//    cmd> $
//    --- 1.2 ---
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//    --- 2.1 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    --- 2.2 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$llo                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$llo                // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//    ==> In either of the following cases, print out cmd + ' '
//    ==> and reset _tabPressCount to 0
//    --- 3.1 ---
//    [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $               // auto completed with a space inserted
//    --- 3.2 ---
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$ahah
//    [After Tab]
//    cmd> heLp $ahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//    --- 3.3 ---
//    [Before] fully matched (cursor right behind cmd)
//    cmd> hElP$sdf
//    [After Tab]
//    cmd> hElP $sdf            // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//    --- 4.1 ---
//    [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//    --- 5.1 ---
//    [Before] Already matched on first tab pressing
//    cmd> help asd$gh
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//    ==> Note: command usage has been printed under first tab press
//    ==> Check the word the cursor is at; get the prefix before the cursor
//    ==> So, this is to list the file names under current directory that
//        match the prefix
//    ==> List all the matched file names alphabetically by:
//           cout << setw(16) << left << fileName;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location
//    --- 6.1 ---
//    [Before] if prefix is empty, print all the file names
//    cmd> help $sdfgh
//    [After]
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.2 ---
//    [Before] with a prefix and with mutiple matched files
//    cmd> help M$Donald
//    [After]
//    Makefile        MustExist.txt   MustRemove.txt
//    cmd> help M$Donald
//    --- 6.3 ---
//    [Before] with a prefix and with mutiple matched files,
//             and these matched files have a common prefix
//    cmd> help Mu$k
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help Must$k
//    --- 6.4 ---
//    [Before] with a prefix and with a singly matched file
//    cmd> help MustE$aa
//    [After] insert the remaining of the matched file name followed by a ' '
//    cmd> help MustExist.txt $aa
//    --- 6.5 ---
//    [Before] with a prefix and NO matched file
//    cmd> help Ye$kk
//    [After] beep and stay in the same location
//    cmd> help Ye$kk
//
//    [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//    --- 7.1 ---
//    [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location

void
CmdParser::listCmd(const string& str)
{
   // TODO...
   bool sp_flag = false;
   string option="";
   for(size_t i=0; i<str.size(); i++)
   {
      if(str[i]==' ')
      {
         if(option=="") continue;
         else 
         {
            sp_flag = true;
            break;
         }
      }
      else option = option + str[i];
   }
   if(option=="") 
   {
      char* tmp = _readBufPtr;
      moveBufPtr(_readBufEnd);
      cout<<endl;
      cout<<"DBAPpend    DBAVerage   DBCount     DBDelete    DBMAx"<<endl      
          <<"DBMIn       DBPrint     DBRead      DBSOrt      DBSUm"<<endl       
          <<"DOfile      HELp        HIStory     Quit"<<endl;
      cout<<"mydb> "<<_readBuf;
      moveBufPtr(tmp);
      _tabPressCount = 0;
   }
   else
   {
      if(sp_flag == false)
      {
         vector<string> vec;
         for(auto it = _cmdMap.begin(); it!=_cmdMap.end(); it++)
         {
            string cmd_name=it->first+it->second->getOptCmd();
            if(option.size()>cmd_name.size()) continue;
            else
            {
               if(!myStrNCmp(cmd_name, option, option.size())) vec.push_back(cmd_name);
            }
         }
         if(vec.size()==1)
         {
            vec[0]=vec[0]+' ';
            //change buffer
            size_t i=0;
            for(; i<str.size(); i++)
            {
               if(str[i]!=' ') break;
            }
            size_t j=_readBufPtr - _readBuf;
            for(size_t k=j-i; k<vec[0].size(); k++)
            {
               insertChar(vec[0][k]);
            }
            _tabPressCount = 0;
         }
         else if(vec.size()>1)
         {
            //print
            char* tmp = _readBufPtr;
            moveBufPtr(_readBufEnd);
            for(size_t i=0; i<vec.size(); i++)
            {
               if(i%5==0) cout<<endl;
               cout << setw(12) << left << vec[i];
            }
            cout<<endl<<"mydb> "<<_readBuf;
            moveBufPtr(tmp);
            _tabPressCount = 0;
         }
         else mybeep();
      }
      else
      {
         if(_tabPressCount==1)
         {
            CmdExec* e = 0;
            //print usage
            for(auto it = _cmdMap.begin(); it!=_cmdMap.end(); it++)
            {
               string cmd_name=it->first+it->second->getOptCmd();
               if(!myStrNCmp(cmd_name, option, (it->first).size())) 
               {
                  e = it->second;
                  break;
               }
            }
            if(e!=0) 
            {
               char* tmp = _readBufPtr;
               moveBufPtr(_readBufEnd);
               cout<<endl;
               e->usage(cout);
               cout<<endl<<"mydb> "<<_readBuf;
               moveBufPtr(tmp);
               _tabPressCount = 1;
            }
            else
            {
            mybeep();
            _tabPressCount = 0;
            }
         }
         else
         {
            //目錄分析
            vector<string> files;
            string prefix;
            for(size_t i=0; i<str.size(); i++)
            {
               if(str[str.size()-1-i]==' ') break;
               else prefix=str[str.size()-1-i]+prefix;
            }
            listDir(files, prefix, ".");
            if(files.size()==0) mybeep();
            else
            {
               size_t i=0;
               size_t k=0;
               for(; i<files[0].size(); i++)
               {
                  for(size_t j=0; j<files.size(); j++)
                  {
                     if(files[j].size()<=i) break;
                     if(files[0][i]!=files[j][i]) break;
                     else k++;
                  }
                  if(k!=files.size()) break;
                  k=0;
               }
               if(i==prefix.size())
               {
                  char* tmp = _readBufPtr;
                  moveBufPtr(_readBufEnd);
                  for(size_t j=0; j<files.size(); j++)
                  {
                     if(j%5==0) cout<<endl;
                     cout << setw(16) << left << files[j];
                  }
                  cout<<endl<<"mydb> "<<_readBuf;
                  moveBufPtr(tmp);
               }
               else
               {
                  char* tmp = _readBufPtr;
                  moveBufPtr(_readBufEnd);
                  cout<<endl<<"mydb> "<<_readBuf;
                  moveBufPtr(tmp);
                  for(size_t j=prefix.size(); j<i; j++)
                  {
                     insertChar(files[0][j]);
                  }

               }
            }
         }
      }
   }   
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
   CmdExec* e = 0;
   // TODO...
   //std::map<const string, CmdExec*>::iterator it;
   for(auto it = _cmdMap.begin(); it!=_cmdMap.end(); it++)
   {
      string str=it->first+it->second->getOptCmd();
      if(!myStrNCmp(str, cmd, (it->first).size()))
      {
         e = it->second;
         break;
      }
   }
   return e;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);
   if (!optional) {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Error: Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Error: Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Error: Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}

