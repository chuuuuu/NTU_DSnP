/****************************************************************************
  FileName     [ p2Table.h]
  PackageName  [ p2 ]
  Synopsis     [ Header file for class Row and Table ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2016-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef P2_TABLE_H
#define P2_TABLE_H

#include <vector>
#include <string>

using namespace std;

class Row
{
public:
   // TODO: define constructor and member functions on your own
   void CreateRow(size_t);
   void BuildData(int, char, bool);
   void Zero(int);
   // Basic access functions
   const int operator[] (size_t i) const { return _data[i]; }
   int& operator[] (size_t i) { return _data[i]; }

private:
   int    *_data; // DO NOT change this definition. Use it to store data.
};

class Table
{
public:
   // TODO: define constructor and member functions on your own
   bool read(const string&);
   bool CheckCol(size_t);
   void print();
   void sum();
   void ave();
   void max();
   void min();
   void dist();
   void add();
   // Basic access functions
   size_t nCols() const { return _nCols; }
   const Row& operator[] (size_t i) const { return _rows[i]; }
   Row& operator[] (size_t i) { return _rows[i]; }

private:
   size_t       _nCols; // You should record the number of columns.
   size_t	_nRows;
   vector<Row>  _rows;  // DO NOT change this definition. Use it to store rows.
   string	_content;
};

#endif // P2_TABLE_H
