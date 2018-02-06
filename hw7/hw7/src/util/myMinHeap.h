/****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap
{
public:
   MinHeap(size_t s = 0) { if (s != 0) _data.reserve(s); }
   ~MinHeap() {}

   void clear() { _data.clear(); }

   // For the following member functions,
   // We don't respond for the case vector "_data" is empty!
   const Data& operator [] (size_t i) const { return _data[i]; }   
   Data& operator [] (size_t i) { return _data[i]; }

   size_t size() const { return _data.size(); }

   // TODO
   const Data& min() const { return _data[0]; }
   void insert(const Data& d) 
   {
      _data.push_back(d);
      size_t tmp = size()-1;
      while(tmp != 0)
      {
         if(_data[tmp]<_data[(tmp-1)/2]) swap(_data[tmp], _data[(tmp-1)/2]);
         else break;
         tmp = (tmp-1)/2;
      }
   }
   void delData(size_t i) 
   {
      swap(_data[i], _data[size()-1]);
      _data.pop_back();
      size_t tmp = i;
      while(tmp!=0)
      {
         if(_data[tmp]<_data[(tmp-1)/2])
         {
            swap(_data[(tmp-1)/2], _data[tmp]);
            tmp = (tmp-1)/2;
         }
         else break;
      }
      while(tmp*2+2<size())
      {
         if(_data[tmp*2+2] < _data[tmp*2+1])
         {
            if(_data[tmp*2+2] < _data[tmp])
            {
               swap(_data[tmp*2+2], _data[tmp]);
               tmp = tmp*2+2;
            }
            else break;
         }
         else
         {
            if(_data[tmp*2+1] < _data[tmp]) 
            {
               swap(_data[tmp*2+1], _data[tmp]);
               tmp = tmp*2+1;
            }
            else break;
         }
      }
      if(tmp*2+1<size())
      {
         if(_data[tmp*2+1] < _data[tmp]) swap(_data[tmp*2+1], _data[tmp]);
      }
   }
   void delMin()
   {
      delData(0);
   }

private:
   // DO NOT add or change data members
   vector<Data>   _data;
};

#endif // MY_MIN_HEAP_H
