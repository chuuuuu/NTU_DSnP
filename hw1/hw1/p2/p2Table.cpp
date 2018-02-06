/****************************************************************************
  FileName     [ p2Table.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define member functions of class Row and Table ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2016-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include "p2Table.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

// Implement member functions of class Row and Table here

void Row::CreateRow(size_t _nCols)
{
   _data = new int(_nCols);
   for(size_t i=0; i<_nCols; i++)
   {
      _data[i] = 2147483647;
   }
}

void Row::BuildData(int index, char c, bool neg_flag)
{
   if(neg_flag) _data[index]=_data[index] * 10 - c + '0';
   else _data[index]=_data[index] * 10 + c - '0';
}

void Row::Zero(int index)
{
   _data[index] = 0;
}

bool Table::read(const string& csvFile)
{
   stringstream str;
   fstream file;
   file.open(csvFile,ios::in);
   if(!file) return false;
   else
   {
      str<<file.rdbuf();
      _content = str.str();
//calculate the column numbers
      int ColNum =0;
         for(size_t i=0; i<_content.size(); i++)
         {
            if(_content[i]!='-')
            {
               if(_content[i]==',') ColNum++;
               else if(_content[i]>57||_content[i]<48)
               {
                  ColNum++;
                  break;
               }
            }
         }
      _nCols = ColNum;
      
//calculate the row numbers
      int CommaNum = 0;
      for(size_t i=0; i<_content.size(); i++)
      {
         if(_content[i]==',') CommaNum++;
      }
      _nRows = CommaNum/(_nCols-1);
//store rowscomparison between signed and unsigned integer expressions [-Wsign-compare]
      int Num = 0;  
      for(size_t i=0; i<_nRows; i++)
      {
         size_t count = 0;
         bool count_flag = true;
         bool neg_flag = false;
         Row _row;
         _row.CreateRow(_nCols);
         while(true)
         {
            if(_content[Num] == '-') neg_flag = true;
            else if(_content[Num]<58 && _content[Num]>47)
            {
               if(count_flag)
               {
                  _row.Zero(count);
                  count_flag = false;
               }
               _row.BuildData(count, _content[Num], neg_flag);
            }
            else if(_content[Num]==',') 
            {
               count++;
               count_flag = true;
               neg_flag = false;
            }
            else if(count == (_nCols-1)) break;
            Num++;
         }
         _rows.push_back(_row);
      }
      return true; // TODO
   }
}

bool Table::CheckCol(size_t col)
{
   for(size_t i=0; i<_nRows; i++)
   { 
      if(_rows[i][col]!=2147483647) return true;
   }
   return false;
}

void Table::print()
{
   for(size_t i=0; i<_nRows; i++)
   {
      for(size_t j=0; j<_nCols; j++)
      {
         if(_rows[i][j] != 2147483647) cout<<setw(4)<<right<<_rows[i][j];
         else cout<<setw(4)<<right<<'.';
      }
      cout<<endl;
   }
}

void Table::sum()
{
   size_t col;
   int SUM = 0;
   cin>>col;
   if(CheckCol(col))
   {
      for(size_t i=0; i<_nRows; i++)
      {
         if(_rows[i][col]!=2147483647) SUM += _rows[i][col];
      }
      cout<<"The summation of data in column #"<<col<<" is "<<SUM<<'.'<<endl;
   }
   else cout<<"Error: This is a NULL column!!"<<endl;
}

void Table::ave()
{
   size_t col;
   int SUM = 0;
   int count = 0;
   cin>>col;
   if(CheckCol(col))
   {
      for(size_t i=0; i<_nRows; i++)
      {
         if(_rows[i][col]!=2147483647)
         {
            count++;
            SUM += _rows[i][col];
         }
      }
      cout<<"The average of data in column #"<<col<<" is "<<fixed<<setprecision(1)<<(float)SUM/count<<'.'<<endl;
   }
   else cout<<"Error: This is a NULL column!!"<<endl;
}

void Table::max()
{
   size_t col;
   int MAX = -2147483648;
   cin>>col;
   if(CheckCol(col))
   {
      for(size_t i=0; i<_nRows; i++)
      {
         if(_rows[i][col]>MAX && _rows[i][col]!=2147483647) MAX=_rows[i][col];
      }
      cout<<"The maximum of data in column #"<<col<<" is "<<MAX<<'.'<<endl;
   }
   else cout<<"Error: This is a NULL column!!"<<endl;
}

void Table::min()
{
   size_t col;
   int MIN = 2147483647;
   cin>>col;
   if(CheckCol(col))
   {
      for(size_t i=0; i<_nRows; i++)
      {
         if(_rows[i][col]<MIN) MIN=_rows[i][col];
      }
      cout<<"The minimum of data in column #"<<col<<" is "<<MIN<<'.'<<endl;
   }
   else cout<<"Error: This is a NULL column!!"<<endl;
}

void Table::dist()
{
   size_t col;
   int count=1;
   cin>>col;
   if(CheckCol(col))
   {
      if(_rows[0][col]==2147483647) count--;
      for(size_t i=1; i<_nRows; i++)
      {
         if(_rows[i][col]!=2147483647)
         {
            count++;
            for(size_t j=0; j<i; j++)
            {
               if(_rows[i][col] == _rows[j][col]) 
               {
                  count--;
                  break;
               }
            }
         }
      }
      cout<<"The distinct count of data in column #"<<col<<" is "<<count<<'.'<<endl;
   }
   else cout<<"Error: This is a NULL column!!"<<endl;
}

void Table::add()
{
   string str;
   size_t Num = 1;
   size_t count = 0;
   bool count_flag = true;
   bool neg_flag = false;
   Row _row;
   _row.CreateRow(_nCols);
   getline(cin,str);
   while(Num<str.size())
   {
      if(str[Num] == '-') neg_flag = true;
      else if(str[Num]<58&&str[Num]>47)
      {
         if(count_flag)
         {
            _row.Zero(count);
            count_flag = false;
         }
         _row.BuildData(count, str[Num], neg_flag);
      }
      else if(str[Num] == ' ')
      {
         count++;
         count_flag = true;
         neg_flag = false;
      }
      Num++;
   }
   _rows.push_back(_row);
   _nRows++;
}
