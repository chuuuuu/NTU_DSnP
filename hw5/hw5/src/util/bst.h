/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
   BSTreeNode() {}
   BSTreeNode(T d, BSTreeNode<T>* pNode=NULL, BSTreeNode<T>* rNode=NULL, BSTreeNode<T>* lNode=NULL):_data(d),  _pNode(pNode), _rNode(rNode), _lNode(lNode) {}
   ~BSTreeNode() {}

   friend class BSTree<T>;
   friend class BSTree<T>::iterator;
   
   T _data;
   BSTreeNode<T>* _pNode;
   BSTreeNode<T>* _rNode;
   BSTreeNode<T>* _lNode;
   // TODO: design your own class!!
};



template <class T>
class BSTree
{
public:
   friend class iterator;
   BSTree()
   {
      _end = new BSTreeNode<T>;
      _end->_pNode = NULL;
      _end->_rNode = NULL;
      _end->_lNode = NULL;
      //_endIt._node = NULL;
      _size = 0;
      _root = NULL;
   }
   ~BSTree()
   {
      clear();
      delete _end;
   }

   // TODO: design your own class!!
   class iterator 
   { 
      friend class BSTree;

   public:
      iterator(BSTreeNode<T>* n=0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {}

      const T& operator * () const {return _node->_data;}
      T& operator * () {return _node->_data;}

      iterator& operator ++ () 
      {
         if(_node->_rNode!=NULL)
         {
            _node = _node->_rNode;
            while(_node->_lNode != NULL)
            {
               _node = _node->_lNode;
            }
            return *this;
         }
         else if(_node->_pNode!=NULL)
         {
            while(_node->_pNode!=NULL && _node->_pNode->_rNode==_node)
            {
               _node = _node->_pNode;
            }
            _node = _node->_pNode;
            return *this;
         }
         _node = NULL;
         return *this;  
      }
      iterator operator ++ (int) {iterator it = *this; operator++(); return it;}
      iterator& operator -- () 
      {
         if(_node->_lNode!=NULL)
         {
            _node = _node->_lNode;
            while(_node->_rNode!=NULL)
            {
               _node = _node->_rNode;
            }
            return *this;
         }
         else if(_node->_pNode!=NULL)
         {
            if(_node->_pNode->_rNode == _node)
            {
               _node = _node->_pNode;
               return *this;
            }
            while(_node->_pNode!=NULL && _node->_pNode->_lNode==_node)
            {
               _node = _node->_pNode;
            }
            _node = _node->_pNode;
            return *this;
         }
         _node = NULL;
         return *this;
      }
      iterator operator -- (int) {iterator it = *this; operator--(); return it;}
      iterator& operator = (const iterator& i) {_node = i._node; return *this;}
      bool operator != (const iterator& i) const {if(i._node != _node) return true; else return false;}
      bool operator == (const iterator& i) const {return !operator!=(i);}

   private:
      BSTreeNode<T>* _node;
   };

   iterator begin() const 
   {
      BSTreeNode<T>* begin = _root;
      if(begin == NULL)
      {
         iterator it(_end);
         return it;
      }
      while(begin->_lNode != NULL) begin = begin->_lNode;
      iterator it(begin);
      return it;
   }
   iterator end() const 
   {
      /*BSTreeNode<T>* end = _root;
      if(end == NULL)
      {
         iterator it(end);
         return it;
      }
      while(end->_rNode != NULL) end = end->_rNode;
      iterator it(end);
      return it;*/
      iterator it(_end);
      return it;
   }
   bool empty() const
   {
      if(_root == NULL) return true;
      else return false;
   }
   size_t size() const {return _size;}
   void insert(const T& x)
   {
      BSTreeNode<T>* node = new BSTreeNode<T>(x);
      BSTreeNode<T>* tmp = _root;
      if(tmp==NULL)
      {
         _root = node;

         _end->_pNode = _root;
         _root->_rNode = _end;
      }
      else
      {
         while(true)
         {
            if(x >= tmp->_data)
            {
               if(tmp->_rNode==_end)
               {
                  tmp->_rNode = node;
                  node->_pNode = tmp;
                  node->_rNode = _end;
                  _end->_pNode = node;
                  break;
               }
               else if(tmp->_rNode==NULL)
               {
                  tmp->_rNode = node;
                  node->_pNode = tmp;
                  break;
               }
               else
               {
                  tmp = tmp->_rNode;
               }
            }
            else if(x < tmp->_data)
            {
               if(tmp->_lNode==NULL)
               {
                  tmp->_lNode = node;
                  node->_pNode = tmp;
                  break;
               }
               else
               {
                  tmp = tmp->_lNode;
               }
            }
         }
      }
      //setEnd();
      _size++;
   }
   bool erase(iterator pos)
   {
      if(pos._node==NULL||pos._node==_end) 
      {
         return false;
      }
      if(pos._node->_rNode != NULL && pos._node->_rNode != _end)
      {
         //cout<<"1"<<endl;
         BSTreeNode<T>* tmp = pos._node->_rNode;
         while(tmp->_lNode != NULL)
         {
            tmp = tmp->_lNode;
         }
         if(tmp->_pNode->_lNode == tmp) 
         {
            tmp->_pNode->_lNode = tmp->_rNode;
            if(tmp->_rNode != NULL) tmp->_rNode->_pNode = tmp->_pNode;
         }
         else 
         {
            tmp->_pNode->_rNode = tmp->_rNode;
         }
         if(tmp->_rNode != NULL) tmp->_rNode->_pNode = tmp->_pNode;
         *pos = tmp->_data;
         delete tmp;
      }
      else if(pos._node->_lNode != NULL)
      {
         //cout<<"2"<<endl;
         if(pos._node == _root)
         {
            _root = _root->_lNode;
            _root->_pNode = NULL;
            setEnd();
            delete pos._node;
         }
         else
         {  
            /*cout<<pos._node->_pNode->_data
                <<pos._node->_pNode->_lNode->_data<<endl
                <<pos._node->_pNode->_rNode->_data<<endl;*/
            if(pos._node->_pNode->_lNode == pos._node) 
            {
               pos._node->_pNode->_lNode = pos._node->_lNode;
            }
            else 
            {
               pos._node->_pNode->_rNode = pos._node->_lNode;
               setEnd();
            }
            pos._node->_lNode->_pNode = pos._node->_pNode;
            delete pos._node;
         }
      }
      else
      {
         //cout<<"3"<<endl;
         if(pos._node == _root)
         {
            delete _root;
            _root = NULL;
            _end->_pNode = NULL;
         }
         else
         {
            if(pos._node->_pNode->_lNode == pos._node) 
            {
               //cout<<"555"<<endl;
               pos._node->_pNode->_lNode = NULL;
            }
            else 
            {
               pos._node->_pNode->_rNode = pos._node->_rNode;
               if(pos._node->_rNode == _end)
               {
                  _end->_pNode = pos._node->_pNode;
               }
            }
            delete pos._node;
         }
      }
      //setEnd();
      //cout<<"789789789"<<endl;
      _size--;
      return true;
   }
   bool erase(const T& x)
   {
      for(auto it=begin(); it!=end(); it++)
      {
         if(*it==x) 
         {
            erase(it);
            return true;
         }
      }
      return false;
   }
   void pop_front()
   {
      erase(begin());
   }
   void pop_back()
   {
      /*BSTreeNode<T>* node = _root;
      if(_root != NULL)
      {
         while(node->_rNode != NULL)
         {
            node = node->_rNode;
         }
         iterator it(node);
         erase(it);
      }*/
      iterator it(_end);
      it--;
      erase(it);
   }
   void clear()
   {
      while(!empty())
      {
         //cout<<"123"<<endl;
         pop_front();
      }
   }
   void sort() const {}
   void print() const
   {
      /*for(auto it=begin(); it!=end(); it++)
      {
         
      }*/
   }
private:
   BSTreeNode<T>* _root;
   BSTreeNode<T>* _end;
   size_t _size;
   void setEnd()
   {
      BSTreeNode<T>* _node = _root;
      if(_node != NULL)
      {
         while(_node->_rNode != NULL && _node->_rNode != _end)
         {
            _node = _node->_rNode;
         }
         _node->_rNode = _end;
         _end->_pNode = _node;
      }
      else
      {
         _end->_pNode = NULL;
      }
     // _endIt._node = _end;
   }
};

#endif // BST_H
