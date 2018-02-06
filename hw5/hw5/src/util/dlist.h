/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() {
      _isSorted = true;
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return this->_node->_data; }
      T& operator * () { return this->_node->_data; }
      iterator& operator ++ () { this->_node = this->_node->_next; return *(this); }
      iterator operator ++ (int) { iterator it = *(this); this->_node = this->_node->_next; return it; }
      iterator& operator -- () { this->_node = this->_node->_prev; return *(this); }
      iterator operator -- (int) { iterator it = *(this); this->_node = this->_node->_prev; return it; }

      iterator& operator = (const iterator& i) { this->_node = i._node; return *(this); }

      bool operator != (const iterator& i) const { if(this->_node == i._node) return false; else return true; }
      bool operator == (const iterator& i) const { if(this->_node == i._node) return true; else return false; }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const { iterator it(_head); return it; }
   iterator end() const { iterator it(_head->_prev); return it; }
   bool empty() const 
   { 
      if(this->begin()==this->end()) return true; 
      else return false;
   }
   size_t size() const { size_t size=0; for(iterator it=begin(); it!=end(); it++){size++;} return size; }

   void push_back(const T& x) 
   { 
      DListNode<T>* node = new DListNode<T>(x, this->_head->_prev->_prev, this->_head->_prev); 
      if(this->empty()) 
      {
         this->_head->_prev = node;
         this->_head->_next = node;
         this->_head = node;
      }
      node->_next->_prev = node;
      node->_prev->_next = node;
      _isSorted = false; 
   }
   void pop_front() 
   { 
      if(!(this->empty()))
      {
         DListNode<T>* node = this->_head; 
         this->_head->_next->_prev = this->_head->_prev;
         this->_head->_prev->_next = this->_head->_next;
         this->_head = this->_head->_next;
         delete node;
      }
   }
   void pop_back()
   {
      if(this->begin()._node->_next == this->end()._node)
      {
         DListNode<T>* node = this->_head;
         this->_head = this->_head->_prev;
         this->_head->_prev = this->_head;
         this->_head->_next = this->_head;
         delete node;
      }
      if(!(this->empty()))
      {
         DListNode<T>* node = this->_head->_prev->_prev;
         this->_head->_prev->_prev->_prev->_next = _head->_prev;
         this->_head->_prev->_prev = _head->_prev->_prev->_prev;
         delete node;
      }
   }

   // return false if nothing to erase
   bool erase(iterator pos) 
   { 
      if(this->_head->_prev == pos._node) return false; 
      DListNode<T>* node = pos._node;
      if(this->_head == pos._node)
      {
         this->_head = this->_head->_next;
      }
      pos._node->_next->_prev = pos._node->_prev; 
      pos._node->_prev->_next = pos._node->_next;
      delete node;
      return true;
   }
   bool erase(const T& x)
   {
      for(iterator it = this->begin(); it!=end(); it++)
      {
         if(*(it) == x)
         {
            DListNode<T>* node = it._node;
            if(node == this->_head)
            {
               this->_head = this->_head->_next;
            }
            it._node->_next->_prev = it._node->_prev;
            it._node->_prev->_next = it._node->_next;
            delete node;
            return true;
         }
      }
      return false;
   }

   void clear() 
   {
      while(!(this->empty()))
      {
         this->pop_front();
      }
      this->_isSorted = true;
   }  // delete all nodes except for the dummy node

   void sort() const 
   {
      if(this->_isSorted == false)
      {
         for(iterator i = this->begin()._node; i != this->end()._node; i++)
         {
            for(iterator j = this->end()._node->_prev; j != i; j--)
            {
               if(*i>*j)
               {
                  T data = *i;
                  *i = *j;
                  *j = data;
               }
            }
         }
      }
      this->_isSorted = true;
   }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
};

#endif // DLIST_H
