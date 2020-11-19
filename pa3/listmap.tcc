// $Id: listmap.tcc,v 1.15 2019-10-30 12:44:53-07 - - $
//Dmitriy Nikitin ID: 1649206 asg3

#include "listmap.h"
#include "debug.h"
#include <iostream>

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename key_t, typename mapped_t, class less_t>
listmap<key_t,mapped_t,less_t>::~listmap() {
   DEBUGF ('l', reinterpret_cast<const void*> (this));
   auto iter = this->begin();
   while(iter != this->end()){
      iter = this->erase(iter);
   }
}

//
// iterator listmap::insert (const value_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::insert (const value_type& pair) {
   DEBUGF ('l', &pair << "->" << pair);
   node* nn;
   auto check = find(pair.first);
   if(this->empty()){
   	nn = new node(anchor(),anchor(),pair);
   	anchor()->next = nn;
   	anchor()->prev = nn;
   }
   else if(check != end()){
      (*check).second = pair.second;
   }
   else{
   	auto it = this->begin();
   	while(it!=this->end()){
	   	if(less(pair.first,(*it).first)){
	   		nn = new node(it.get_node(),it.get_node()->prev,pair);
	   		it.get_node()->prev->next = nn;
	   		it.get_node()->prev = nn;
	   		break;
	   	}
	   	else if(next_end(it)){
	   		nn = new node(anchor(),it.get_node(),pair);
	   		it.get_node()->next = nn;
	   		anchor()->prev = nn;
	   		break;
	   	}
	   	else ++it;
   	}
   }
   return nn;
}

//
// listmap::find(const key_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::find (const key_type& that) {
   DEBUGF ('l', that);
   auto it = begin();
   while(it != end()){
      if(!(less((*it).first,that)) && !(less(that,(*it).first)))
      	return it;
      ++it;
   }
   return it;
}

//
// iterator listmap::erase (iterator position)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::erase (iterator position) {
   DEBUGF ('l', &*position);
   auto it = position.get_node()->next;
   auto prev_node = position.get_node()->prev;
   prev_node->next = it;
   it->prev = prev_node;
   delete(position.get_node());
   return it;
}


