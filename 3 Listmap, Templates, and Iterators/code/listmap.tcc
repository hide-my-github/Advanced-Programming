// $Id: listmap.tcc,v 1.7 2015-04-28 19:22:02-07 - - $

#include "listmap.h"
#include "trace.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (node* next, node* prev,
   const value_type& value):
            link (next, prev), value (value) {
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap() {
   TRACE ('l', (void*) this);
   //cout<<"deleting"<<endl;
   for (auto iter = begin(); iter != end();){
	   auto temp = iter;
	   ++iter;
	   erase(temp);
   }
   //cout<<"end of deleting"<<endl;
}


//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::insert (const value_type& pair) {
	//cout<<"---------------INSERTING "<<pair.first<<" & "<<pair.second<<"-----------------"<<endl;
   TRACE ('l', &pair << "->" << pair);
   auto itor = begin();
   if (itor == end()) {  //for creating head only
	//cout << "creating a new head because the list is empty..." << endl;
    node *new_node = new node(begin().where, end().where, pair);
    begin().where->next = new_node;
    begin().where->prev = end().where;
    end().where->next = begin().where;
    end().where->prev = new_node;
	//cout<<"PAIR.FIRST: "<<pair.first<<endl;
	//cout<<"PAIR.SECOND: "<<pair.second<<endl;
	//cout<<":: "<<new_node->value.second<<endl;
    //cout << "(*begin()).first: " << (*begin()).first <<"; (*begin()).second: " << (*begin()).second<<endl;
    //cout << "(*(end()++)).first " << (*(++end())).first << endl;
   	//return itor;
   } else {
    //we already have a big enough list to compare multiple nodes
   	for (; itor != end(); ++itor) {
		//cout<<"FIRST1: "<<(*itor).first<<"; SECOND1: "<<(*itor).second<<endl;
   		if (less(pair.first, (*itor).first)) {
			node *new_node = new node(itor.where, itor.where->prev, pair);
			itor.where->prev->next = new_node;
			itor.where->prev= new_node;

   			return --itor;
   		}
   	}
	//cout<<"INSERTED IN THE END WHERE IT BELONGS..."<<endl;
	//got out of forloop, thus what is to be inserted, must be in the end.
	node *new_node = new node(end().where, end().where->prev, pair);
	--itor; //move out of end()
	itor.where->next = new_node;
	end().where->prev = new_node;
	++itor;
   }
	return itor;
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that) { //const {
   TRACE ('l', that);
   auto seek = begin();
   for(; seek != end(); ++seek) {
   	if(seek->first == that) {
   		cout << seek->first << " = " << seek->second << endl;
   		break;
   	}
   }
   return seek;
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {
   TRACE ('l', &*position);
   auto current = position.where;
   ++position;
   current->prev->next = current->next;
   current->next->prev = current->prev;
   delete current;
   return iterator();
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type&
listmap<Key,Value,Less>::iterator::operator*() {
   TRACE ('l', where);
   return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type*
listmap<Key,Value,Less>::iterator::operator->() {
   TRACE ('l', where);
   return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++() {
   TRACE ('l', where);
   where = where->next;
   return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator--() {
   TRACE ('l', where);
   where = where->prev;
   return *this;
}

//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator==
            (const iterator& that) const {
   return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator!=
            (const iterator& that) const {
   return this->where != that.where;
}
