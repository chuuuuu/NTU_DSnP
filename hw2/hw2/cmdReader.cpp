/****************************************************************************
  FileName     [ cmdReader.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command line reader member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <cstring>
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();
char mygetc(istream&);
ParseChar getChar(istream&);


//----------------------------------------------------------------------
//    Member Function for class Parser
//----------------------------------------------------------------------
void
CmdParser::readCmd()
{
   if (_dofile.is_open()) {
      readCmdInt(_dofile);
      _dofile.close();
   }
   else
      readCmdInt(cin);
}

void
CmdParser::readCmdInt(istream& istr)
{
   resetBufAndPrintPrompt();

   while (1) {
      ParseChar pch = getChar(istr);
      if (pch == INPUT_END_KEY) break;
      switch (pch) {
         case LINE_BEGIN_KEY :
         case HOME_KEY       : moveBufPtr(_readBuf); break;
         case LINE_END_KEY   :
         case END_KEY        : moveBufPtr(_readBufEnd); break;
         case BACK_SPACE_KEY : /* TODO */
                               moveBufPtr(_readBufPtr - 1);
                               deleteChar(); break;
         case DELETE_KEY     : deleteChar(); break;
         case NEWLINE_KEY    : addHistory();
                               cout << char(NEWLINE_KEY);
                               resetBufAndPrintPrompt(); break;
         case ARROW_UP_KEY   : moveToHistory(_historyIdx - 1); break;
         case ARROW_DOWN_KEY : moveToHistory(_historyIdx + 1); break;
         case ARROW_RIGHT_KEY: /* TODO */ 
                               moveBufPtr(_readBufPtr+1); break;
         case ARROW_LEFT_KEY : /* TODO */ 
                               moveBufPtr(_readBufPtr-1); break;
         case PG_UP_KEY      : moveToHistory(_historyIdx - PG_OFFSET); break;
         case PG_DOWN_KEY    : moveToHistory(_historyIdx + PG_OFFSET); break;
         case TAB_KEY        : /* TODO */ 
                               insertChar(' ', 8-(_readBufPtr-_readBuf)%TAB_POSITION); break;
         case INSERT_KEY     : // not yet supported; fall through to UNDEFINE
         case UNDEFINED_KEY:   mybeep(); break;
         default:  // printable character
            insertChar(char(pch)); break;
      }
      #ifdef TA_KB_SETTING
      taTestOnly();
      #endif
   }
}


// This function moves _readBufPtr to the "ptr" pointer
// It is used by left/right arrowkeys, home/end, etc.
//
// Suggested steps:
// 1. Make sure ptr is within [_readBuf, _readBufEnd].
//    If not, make a beep sound and return false. (DON'T MOVE)
// 2. Move the cursor to the left or right, depending on ptr
// 3. Update _readBufPtr accordingly. The content of the _readBuf[] will
//    not be changed
//
// [Note] This function can also be called by other member functions below
//        to move the _readBufPtr to proper position.
bool
CmdParser::moveBufPtr(char* const ptr)
{
   // TODO...
   if(ptr>=_readBuf && ptr<=_readBufPtr)
   {
      size_t i = (_readBufPtr - ptr);
      for(size_t j=0; j<i; j++) 
         cout<<'\b';
      _readBufPtr = ptr;
   }
   else if(ptr>_readBufPtr && ptr<=_readBufEnd)
   {
      size_t i = (ptr - _readBufPtr);
      for(size_t j=0; j<i; j++) 
         cout<<*(_readBufPtr+j);
      _readBufPtr = ptr;
   }
   else 
   {
      mybeep();
      return false;
   }
   return true;
} 


// [Notes]
// 1. Delete the char at _readBufPtr
// 2. mybeep() and return false if at _readBufEnd
// 3. Move the remaining string left for one character
// 4. The cursor should stay at the same position
// 5. Remember to update _readBufEnd accordingly.
// 6. Don't leave the tailing character.
// 7. Call "moveBufPtr(...)" if needed.
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteChar()---
//
// cmd> This is he command
//              ^
//
bool
CmdParser::deleteChar()
{
   size_t i = (_readBufEnd - _readBufPtr);
   if(i == 0)
   {
      mybeep();
      return false;
   }
   else
   {
      for(size_t j=1; j<i; j++)
      {
         cout<<*(_readBufPtr+j);
         *(_readBufPtr+j-1) = *(_readBufPtr+j);
      }
      cout<<' ';
      for(size_t j=0;j<i;j++) cout<<'\b';
      _readBufEnd--;
      *_readBufEnd = 0;
   // TODO...
      return true;
   }
}

// 1. Insert character 'ch' for "repeat" times at _readBufPtr
// 2. Move the remaining string right for "repeat" characters
// 3. The cursor should move right for "repeats" positions afterwards
// 4. Default value for "repeat" is 1. You should assert that (repeat >= 1).
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling insertChar('k', 3) ---
//
// cmd> This is kkkthe command
//                 ^
//
void
CmdParser::insertChar(char ch, int repeat)
{
   // TODO...
   assert(repeat >= 1);
   int i = _readBufEnd - _readBufPtr;
   _readBufEnd += repeat;
   *(_readBufEnd) = 0;
   for(int j = 0; j<repeat; j++)
   {
      cout<<ch;
   }
   for(int j = 0; j<i; j++)
   {
      cout<<*(_readBufPtr+j);
   }
   for(int j = 1; j<=i; j++)
   {
      *(_readBufEnd - j)=*(_readBufEnd - j - repeat);
   }
   for(int j = 0; j<repeat; j++)
   {
      *(_readBufEnd - i - 1 - j) = ch;
   }
   _readBufPtr += repeat;
   for(int j = 0; j<i; j++)
   {
      cout<<'\b';
   }
}

// 1. Delete the line that is currently shown on the screen
// 2. Reset _readBufPtr and _readBufEnd to _readBuf
// 3. Make sure *_readBufEnd = 0
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteLine() ---
//
// cmd>
//      ^
//
void
CmdParser::deleteLine()
{
   size_t i = _readBufPtr - _readBuf;
   for(size_t j=0; j<i; j++) cout<<'\b';
   size_t j = _readBufEnd - _readBuf;
   for(size_t k=0; k<j; k++) cout<<' ';
   for(size_t k=0; k<j; k++) cout<<'\b';

   _readBufPtr = _readBuf;
   _readBufEnd = _readBuf;
   *_readBufEnd = 0;

   // TODO...
}


// This functions moves _historyIdx to index and display _history[index]
// on the screen.
//
// Need to consider:
// If moving up... (i.e. index < _historyIdx)
// 1. If already at top (i.e. _historyIdx == 0), beep and do nothing.
// 2. If at bottom, temporarily record _readBuf to history.
//    (Do not remove spaces, and set _tempCmdStored to "true")
// 3. If index < 0, let index = 0.
//
// If moving down... (i.e. index > _historyIdx)
// 1. If already at bottom, beep and do nothing
// 2. If index >= _history.size(), let index = _history.size() - 1.
//
// Assign _historyIdx to index at the end.
//
// [Note] index should not = _historyIdx
//
void
CmdParser::moveToHistory(int index)
{
   // TODO...
   if(index < _historyIdx)
   {
      if(_historyIdx <= 0)
      {
         mybeep();
      }
      else
      {
         if(_historyIdx == int(_history.size()) && _tempCmdStored == false)
         {
            string str1(_readBuf);
            _history.push_back(str1);
            _tempCmdStored = true;
         }
         if(_historyIdx == int(_history.size()-1) && _tempCmdStored == true)
         {
            string str1(_readBuf);
            _history.pop_back();
            _history.push_back(str1);
         }
         if(index<0) index=0;
         _historyIdx = index;
         retrieveHistory();
      }
   }
   else
   {
      if(_historyIdx >= int(_history.size()-1))
      {
         mybeep();
      }
      else
      {
         if(index > int(_history.size()-1)) index = int(_history.size()-1);
         _historyIdx = index;
         retrieveHistory();
      }
   }
}


// This function adds the string in _readBuf to the _history.
// The size of _history may or may not change. Depending on whether 
// there is a temp history string.
//
// 1. Remove ' ' at the beginning and end of _readBuf
// 2. If not a null string, add string to _history.
//    Be sure you are adding to the right entry of _history.
// 3. If it is a null string, don't add anything to _history.
// 4. Make sure to clean up "temp recorded string" (added earlier by up/pgUp,
//    and reset _tempCmdStored to false
// 5. Reset _historyIdx to _history.size() // for future insertion
//
void
CmdParser::addHistory()
{
   string str1(_readBuf);
   // TODO...
   for(size_t i=0; i<str1.size(); i++)
   {
      if(str1[0]==' ') str1.erase(str1.begin());
      else break;
   }
   for(size_t i=0; i<str1.size();i++)
   {
      if(str1[str1.size()-1]==' ') str1.erase(str1.end()-1);
      else break;
   }
   if(_tempCmdStored)
   {
      _history.pop_back();
   }

   if(!str1.empty())
   {
      _history.push_back(str1);
   }
   _tempCmdStored = false;
   _historyIdx = int(_history.size());
}


// 1. Replace current line with _history[_historyIdx] on the screen
// 2. Set _readBufPtr and _readBufEnd to end of line
//
// [Note] Do not change _history.size().
//
void
CmdParser::retrieveHistory()
{
   deleteLine();
   strcpy(_readBuf, _history[_historyIdx].c_str());
   cout << _readBuf;
   _readBufPtr = _readBufEnd = _readBuf + _history[_historyIdx].size();
}
