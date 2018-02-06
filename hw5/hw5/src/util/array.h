/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>

using namespace std;

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
   // TODO: decide the initial value for _isSorted
   Array() : _data(0), _size(0), _capacity(0) { _isSorted = true; }
   ~Array() { delete [] _data; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class Array;

   public:
      iterator(T* n= 0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return *(this->_node); }
      T& operator * () { return *(this->_node); }
      iterator& operator ++ () { this->_node = this->_node + 1; return *(this); }
      iterator operator ++ (int) { iterator it = *this; this->_node = this->_node + 1; return it; }
      iterator& operator -- () { this->_node = this->_node - 1; return *(this); }
      iterator operator -- (int) { iterator it = *this; this->_node = this->_node - 1; return it; }

      iterator operator + (int i) const { iterator it = *(this); it._node += i; return it; }
      iterator& operator += (int i) { this->_node = this->_node + i; return *(this); }

      iterator& operator = (const iterator& i) { this->_node = i->_node; return *(this); }

      bool operator != (const iterator& i) const { if(this->_node != i._node) return true; else return false; }
      bool operator == (const iterator& i) const { if(this->_node == i._node) return true; else return false; }

   private:
      T*    _node;
   };

   // TODO: implement these functions
   iterator begin() const { iterator it(_data); return it; }
   iterator end() const { iterator it(_data+_size); return it; }
   bool empty() const { if(_size == 0) return true; else return false; }
   size_t size() const { return _size; }

   T& operator [] (size_t i) { return _data[i]; }
   const T& operator [] (size_t i) const { return _data[i]; }

   void push_back(const T& x) 
   { 
      if(_size == _capacity)
      {
         if(_size == 0)
         {
            T* data = new T[1];
            _capacity = 1;
            _data = data;
         }
         else
         {
            T* data = new T[_capacity*2];
            for(size_t i=0; i<_size; i++)
            {
               data[i] = this->_data[i];
            }
            delete [] _data;
            this->_data = data;
            _capacity = _capacity * 2;
         }
      }
      _data[_size] = x;
      _size++;
      _isSorted = false;
   }
   void pop_front() 
   { 
      if(_size>=0)
      {
         _data[0] = _data[_size-1]; 
         _size--; 
         _isSorted = false; 
      }
   }
   void pop_back() { if(_size>=0) _size--; }

   bool erase(iterator pos) 
   { 
      if(pos._node >= this->end()._node || pos._node < this->begin()._node) 
      {
         cout<<"F"<<endl;
         return false;
      }

      *pos = _data[_size-1]; 
      _size--; 
      _isSorted = false; 
      return true;
   }
   bool erase(const T& x)
   {
      for(iterator it = this->begin(); it != this->end(); it++)
      {
         if(*it == x)
         {
            erase(it);
            _isSorted = false;
            return true;
         }
      }
      return false;
   }
   void clear() { _size = 0; _isSorted = true;}

   // [Optional TODO] Feel free to change, but DO NOT change ::sort()
   void sort() const { if (_isSorted == false) ::sort(_data, _data+_size); }

   // Nice to have, but not required in this homework...
   // void reserve(size_t n) { ... }
   // void resize(size_t n) { ... }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   T*            _data;
   size_t        _size;       // number of valid elements
   size_t        _capacity;   // max number of elements
   mutable bool  _isSorted;   // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] Helper functions; called by public member functions
};


#endif // ARRAY_H
