#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <msgpack.hpp>
#include <memory>
#include <cstdatomic>
#include <vector>
#include <assert.h>

#include "markable_ptr.hpp"
#include "util.h"
#include "alloc.h"

#define SL_MAX_LEVEL 16
#define SL_LEVEL_THRES 4
namespace msgskip{
template <
	typename key,typename value,
	typename comparator = std::less<key>,
	typename alloc = std::allocator<char> >
class skiplist{
	typedef skiplist<key,value,comparator,alloc> skiplist_t;
	class node{
		uint32_t max_level_;
		misc::markable_ptr<node>* next_; // array of next pointer
		char packed_key_value_[1]; // msgpack packed
	public:
		explicit node(uint8_t max_level):max_level_(max_level){}
		char* buff(){return packed_key_value_;}
		const char* buff()const{return packed_key_value_;}
		key get_key()const{
			msgpack::object obj;
			msgpack::zone z;
			size_t offset = 0;
			msgpack::unpack_return ret
				= msgpack::unpack(buff(), 8, &offset, &z, &obj);
			return obj.as<key>();
		}
		uint32_t level(){return max_level_;}
		misc::markable_ptr<node>* next(){return next_;}
		/*
			bool compare_and_exchange(int level, node* expected, node* newone){
			return next_[level].compare_exchange_strong(expected, newone);
		}
		node* get_next_relaxed(int level)const{
			if(max_level_ <= level){ return NULL; }
			return next_[level].load(std::memory_order_relaxed);
		}
		node* get_next_acquire(int level)const{
			if(max_level_ <= level){ return NULL; }
			return next_[level].load(std::memory_order_acquire);
		}
		void set_next_relaxed(int level, node* p)const{
			return next_[level].store(p,std::memory_order_relaxed);
		}
		void set_next_release(int level, node* p)const{
			return next_[level].store(p,std::memory_order_release);
		}
		*/
	};
public:
	skiplist()
		:head_(new_node(NULL,NULL,SL_MAX_LEVEL)){}
private:
	// key and value are nullable
	node* new_node(const key* k, const value* v, uint8_t max_level){
		node* memory = reinterpret_cast<node*>(
			arena_.allocate(sizeof(uint32_t) 
											+ max_level * sizeof(std::atomic<node*>)
											+ ((k == NULL) ? 1 : misc::estimate_packed_size(*k))
											+ ((v == NULL) ? 1 : misc::estimate_packed_size(*v)))
		);
		size_t offset = 0;
		if(k){
			msgpack::sbuffer sb;
			msgpack::pack(sb, *k);
			offset += sb.size();
			memcpy(memory->buff(), sb.data(), sb.size());
		}else{
			msgpack::sbuffer sb;
			msgpack::pack(sb, NULL);
			offset += sb.size();
			memcpy(memory->buff(), sb.data(), sb.size());
		}
		if(v){
			msgpack::sbuffer sb;
			msgpack::pack(sb, *v);
			memcpy(&memory->buff()[offset], sb.data(), sb.size());
		}else{
			msgpack::sbuffer sb;
			msgpack::pack(sb, NULL);
			memcpy(memory->buff(), sb.data(), sb.size());
		}
		return memory;
	}
public:	
	bool insert(const key& k, const value& v){
		const int toplevel = rand() % SL_MAX_LEVEL;
		node* preds[SL_MAX_LEVEL+1],*succs[SL_MAX_LEVEL+1];
		while(true){
			if(find(k, preds, succs)){return false;}
			// not found so begin to insert
			node* newnode = new_node(&k,&v,toplevel);
			for(int i = 0; i <= toplevel; ++i){
				newnode->next()[i].store_relaxed(succs[i]);
			}
			if(!preds[0]->next()[0].compare_and_set(succs[0], newnode)){continue;}
			for(int i=1; i <= toplevel; ++i){
				while(true){
					if(preds[i]->next()[0].compare_and_set(succs[i], newnode)){
						break;
					}
					if(find(k, preds, succs)){return false;}
				}
			}
		}
	}
	bool find(const key& k, node** prevs, node** succs)const{
	retry:
		while(true){
			node* pred = head_, *curr, *succ = NULL;
			for(int level = SL_MAX_LEVEL; 0 <= level; --level){
				curr = pred->next()[level].load_acquire();
				while(true){
					if (curr == NULL){continue;}
					succ = curr->next()[level].load_acquire();
					if(curr->get_key() < k){
						pred = curr; curr = succ;
					}else{
						break;
					}
				}
				prevs[level] = pred;
				succs[level] = succ;
			}
			return (curr->get_key() == k);
		}
	}
	misc::single_thread_arena<alloc> arena_;
	comparator compare_;
	node* head_;
};


}
#endif
