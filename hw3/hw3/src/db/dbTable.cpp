/****************************************************************************
  FileName     [ dbTable.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Table member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <string>
#include <cctype>
#include <cassert>
#include <set>
#include <algorithm>
#include "dbTable.h"
#include "util.h"


using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream& operator << (ostream& os, const DBRow& r)
{
   // TODO: to print out a row.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
   for(size_t i=0; i<r.size(); i++)
   {
      if(r[i] == INT_MAX) os << '.';
      else os << r[i];
      if(i == r.size()) os<<endl;
      else os<<' ';
   }
   return os;
}

ostream& operator << (ostream& os, const DBTable& t)
{
   // TODO: to print out a table
   // - Data are seperated by setw(6) and aligned right.
   // - Null cells are printed as '.'
   for(size_t i=0; i<t.nRows(); i++)
   {
      for(size_t j=0; j<t.nCols(); j++)
      {
         if(t[i][j]==INT_MAX)
            os<<setw(6)<<right<<'.';
         else
            os<<setw(6)<<right<<t[i][j];
      }
      os<<endl;
   }
   cout<<endl;
   return os;
}

ifstream& operator >> (ifstream& ifs, DBTable& t)
{
   // TODO: to read in data from csv file and store them in a table
   // - You can assume the input file is with correct csv file format
   // - NO NEED to handle error file format
   ifs.seekg(0,ifs.end);
   size_t len = ifs.tellg();
   ifs.seekg(0, ifs.beg);
   char* buf = new char[len];
   ifs.read(buf, len);
   
   size_t nCols=0;
   for(size_t i=0; i<len; i++)
   {
      if(buf[i]==',') nCols++;
      else if(!(buf[i]=='-'||(buf[i]<='9'&&buf[i]>='0'))) break;
   }
   nCols++;

   size_t nRows=0;
   for(size_t i=0; i<len; i++)
      if(buf[i]==',') nRows++;
   nRows = nRows / (nCols-1);

   size_t x=0;
   for(size_t i=0; i<nRows; i++)
   {  
      DBRow r;
      int neg_flag = 1;
      bool count_flag = false;
      int tmp = 0;
      while(x<len)
      {
         if(buf[x]=='-') neg_flag = -1;
         else if(buf[x]<='9' && buf[x]>='0')
         {
            tmp = int(buf[x]-'0') + 10*tmp;
            count_flag = true;
         }
         else if(buf[x]==',')
         {
            if(count_flag == true) r.addData(neg_flag * tmp);
            else r.addData(INT_MAX);
            tmp = 0;
            count_flag = false;
            neg_flag = 1;
         }
         else if(r.size()==(nCols-1)) break;
         x++;
      }
      if(count_flag == true) r.addData(neg_flag * tmp);
      else r.addData(INT_MAX);
      t.addRow(r);
      r.reset();
   }
   return ifs;
}

/*****************************************/
/*   Member Functions for class DBRow    */
/*****************************************/
void
DBRow::removeCell(size_t c)
{
   // TODO
   _data.erase(_data.begin()+c);
}

/*****************************************/
/*   Member Functions for struct DBSort  */
/*****************************************/
bool
DBSort::operator() (const DBRow& r1, const DBRow& r2) const
{
   // TODO: called as a functional object that compares the data in r1 and r2
   //       based on the order defined in _sortOrder 
   //       if r1 is in front of r2 than return true
   for(size_t i=0; i<_sortOrder.size(); i++)
   {
      if(r1[_sortOrder[i]]<r2[_sortOrder[i]]) return true;
      else if(r1[_sortOrder[i]]>r2[_sortOrder[i]]) return false;
   }
   return true;
}

/*****************************************/
/*   Member Functions for class DBTable  */
/*****************************************/
void
DBTable::reset()
{
   // TODO
   for(size_t i=0; i<_table.size(); i++)
   {
      _table[i].reset();
   }
   _table.clear();
}

void
DBTable::addCol(const vector<int>& d)
{
   // TODO: add a column to the right of the table. Data are in 'd'.
   for(size_t i=0; i<_table.size(); i++)
   {
      _table[i].addData(d[i]);
   }
}

void
DBTable::delRow(int c)
{
   // TODO: delete row #c. Note #0 is the first row.
   _table.erase(_table.begin()+c);
}

void
DBTable::delCol(int c)
{
   // delete col #c. Note #0 is the first row.
   for (size_t i = 0, n = _table.size(); i < n; ++i)
      _table[i].removeCell(c);
}

// For the following getXXX() functions...  (except for getCount())
// - Ignore null cells
// - If all the cells in column #c are null, return NAN
// - Return "float" because NAN is a float.
float
DBTable::getMax(size_t c) const
{
   // TODO: get the max data in column #c
   bool NAN_flag = true;
   int tmp = INT_MIN;
   for(size_t i=0; i<_table.size(); i++)
   {
      if(_table[i][c]!=INT_MAX)
      {
         NAN_flag = false;
         if(_table[i][c] > tmp)
            tmp = _table[i][c];
      }
   }
   if(NAN_flag) return NAN;
   else return float(tmp);
}

float
DBTable::getMin(size_t c) const
{
   // TODO: get the min data in column #c
   bool NAN_flag = true;
   int tmp = INT_MAX;
   for(size_t i=0; i<_table.size(); i++)
   {
      if(_table[i][c]!=INT_MAX)
      {
         NAN_flag = false;
         if(_table[i][c] < tmp)
            tmp = _table[i][c];
      }
   }
   if(NAN_flag) return NAN;
   else return float(tmp);
}

float 
DBTable::getSum(size_t c) const
{
   // TODO: compute the sum of data in column #c
   bool NAN_flag = true;
   int tmp = 0;
   for(size_t i=0; i<_table.size(); i++)
   {
      if(_table[i][c]!=INT_MAX)
      {
         NAN_flag = false;
         tmp += _table[i][c];
      }
   }
   if(NAN_flag) return NAN;
   else return float(tmp);
}

int
DBTable::getCount(size_t c) const
{
   // TODO: compute the number of distinct data in column #c
   // - Ignore null cells
   int tmp = 0;
   for(size_t i=0; i<_table.size(); i++)
   {
      if(i == 0)
      {
         if(_table[0][c] != INT_MAX) tmp++;
         i++;
      }
      for(size_t j=0; j<i; j++)
      {
         if(_table[i][c] == INT_MAX) break;
         if(_table[i][c] == _table[j][c]) break;
         if(j == i-1) tmp++;
      }
   }
   return tmp;
}

float
DBTable::getAve(size_t c) const
{
   // TODO: compute the average of data in column #c
   int tmp = 0;
   int count = 0;
   bool NAN_flag = true;
   for(size_t i=0; i<_table.size(); i++)
   {
      if(_table[i][c]!=INT_MAX)
      {
         NAN_flag = false;
         tmp += _table[i][c];
         count++;
      }
   }
   if(NAN_flag) return NAN;
   else return float(tmp)/float(count);
   return 0.0;
}

void
DBTable::sort(const struct DBSort& s)
{
   // TODO: sort the data according to the order of columns in 's'
   for(size_t i=1; i<_table.size(); i++)
   {
      size_t tmp = i;
      for(size_t j=i; j!=0; j--)
      {
         if(s(_table[tmp],_table[j-1]))
         {
            DBRow r(_table[tmp]);
            _table[tmp] = _table[j-1];
            _table[j-1] = r;
            tmp = j-1;
         }
         else break;
      }
   }
}

void
DBTable::printCol(size_t c) const
{
   // TODO: to print out a column.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
   for(size_t i=0; i<_table.size(); i++)
   {
      if(_table[i][c]==INT_MAX) cout<<'.';
      else cout<<_table[i][c];
      if(i!=_table.size()-1) cout<<' ';
      else cout<<endl;
   }
}

void
DBTable::printSummary() const
{
   size_t nr = nRows(), nc = nCols(), nv = 0;
   for (size_t i = 0; i < nr; ++i)
      for (size_t j = 0; j < nc; ++j)
         if (_table[i][j] != INT_MAX) ++nv;
   cout << "(#rows, #cols, #data) = (" << nr << ", " << nc << ", "
        << nv << ")" << endl;
}

