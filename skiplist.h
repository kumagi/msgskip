#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <msgpack.hpp>
#include <memory>
#include <cstdatomic>
#include <vector>
#include <random>
#include <assert.h>

#include "markable_ptr.hpp"
#include "util.h"
#include "alloc.h"

#define SL_MAX_LEVEL 16
#define SL_LEVEL_THRES 4
#define OFFSET_OF(x,y) reinterpret_cast<size_t>(&reinterpret_cast<x>(NULL)->y)

namespace msgskip{
template <
	typename key,typename value,
	typename comparator = std::less<key>,
	typename alloc = std::allocator<char> >
class skiplist{
	typedef skiplist<key,value,comparator,alloc> skiplist_t;
	class node{
		uint8_t max_level_;// 0 means one pointer
		misc::markable_ptr<node> next_[1]; // array of next pointer
	public:
		explicit node(uint8_t max_level):max_level_(max_level){}
		char* buff(){return reinterpret_cast<char*>(next_) 
				+ (max_level_ + 1) * sizeof(node*);
		}
		const char* buff()const{return reinterpret_cast<const char*>(next_) 
				+ (max_level_ + 1) * sizeof(node*);
		}
		key get_key()const{
			msgpack::object obj;
			msgpack::zone z;
			size_t offset = 0;
			msgpack::unpack_return ret
				= msgpack::unpack(buff(), 8, &offset, &z, &obj);
			if(ret){}
			return obj.as<key>();
		}
		uint8_t level()const{return max_level_;}
		misc::markable_ptr<node>& next(size_t index){
			assert(index <= max_level_);
			return next_[index];
		}
		const misc::markable_ptr<node>& next(size_t index)const{
			assert(index <= max_level_);
			return next_[index];
		}
	};
public:
	skiplist()
		:head_(new_node(NULL,NULL,SL_MAX_LEVEL)){}
private:
	// key and value are nullable
	node* new_node(const key* k, const value* v, uint8_t max_level){
		node* const memory = reinterpret_cast<node*>(
			arena_.allocate(sizeof(uint32_t)
											+ (max_level + 1) * sizeof(std::atomic<node*>)
											+ ((k == NULL) ? 1 : misc::estimate_packed_size(*k))
											+ ((v == NULL) ? 1 : misc::estimate_packed_size(*v))
			)
		);
		new (memory) node(max_level);
		for(int i = 0;i<=max_level; ++i){
			memory->next(i).store_relaxed(NULL);
		}
		char* const ptr = memory->buff();
		size_t offset = 0;
		offset += misc::pack_it(&ptr[offset], k);
		offset += misc::pack_it(&ptr[offset], v);
		return memory;
	}
public:	
	bool insert(const key& k, const value& v){
		const int toplevel = random();
		node* preds[SL_MAX_LEVEL+1];
		node* succs[SL_MAX_LEVEL+1];
		node* newnode = NULL;
		while(true){
			if(find(k, preds, succs)){return false;}
			// not found so begin to insert
			
			if(!newnode){
				newnode = new_node(&k,&v,toplevel);
			}
			/*
				for(int i=0;i<SL_MAX_LEVEL; ++i){
				printf("%d: %p(%p) -> %p \n", i, preds[i], preds[i]->next(i).load_acquire(), succs[i]);
			}
			*/

			newnode->next(0).store_relaxed(succs[0]);
			if(!preds[0]->next(0).compare_and_set(succs[0], newnode)){continue;}
			for(int i=1; i <= toplevel; ++i){
				while(true){
					newnode->next(i).store_relaxed(succs[i]);
					assert(preds[i]->next(i).load_relaxed() == succs[i]);
					if(preds[i]->next(i).compare_and_set(succs[i], newnode)){
						break;
					}
					find(k, preds, succs);
				}
			}
			break;
		}
		return true;
	}
	bool contains(const key& k)const{
		while(true){
			node* pred = head_, *curr, *succ = NULL;
			for(int level = SL_MAX_LEVEL - 1; 0 <= level; --level){
				curr = pred->next(level).load_acquire();
				while(true){
					if (curr == NULL){succ = NULL; break;}
					succ = curr->next(level).load_acquire();
					if(curr->get_key() < k){
						pred = curr; curr = succ;
					}else{
						break;
					}
				}
			}
			return curr != NULL ? curr->get_key() == k : false;
		}
	}
	void dump()const{
		for(int i=SL_MAX_LEVEL-1;i >= 0 ; --i){
			int cnt = -1;
			node* p = head_;
			printf("%2d: ", i);
			while(p != NULL){
				printf("-> [%d] ", p->get_key());
				p = p->next(i).load_relaxed();
				cnt++;
			}
			printf("  items:%d\n",cnt);
		}
	}
	size_t random()const{
		size_t level = 0;
		while((level < SL_MAX_LEVEL-1) && (random_() & 3) == 0){++level;}
		return level;
	}
	bool find(const key& k, node** preds, node** succs)const{
		while(true){
			node* pred = head_, *curr, *succ = NULL;
			for(int level = SL_MAX_LEVEL-1; 0 <= level; --level){
				curr = pred->next(level).load_acquire();
				while(true){
					if (curr == NULL){break;}
					succ = curr->next(level).load_acquire();
					if(curr->get_key() < k){
						pred = curr; curr = succ;
					}else{
						break;
					}
				}
				assert(pred->next(level).load_acquire() == curr);
				preds[level] = pred;
				succs[level] = curr;
			}
			return curr != NULL ? (curr->get_key() == k) : false;
		}
	}
	misc::single_thread_arena<alloc> arena_;
	mutable std::mt19937 random_;
	comparator compare_;
	node* head_;
};


}
#endif
