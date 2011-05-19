#ifndef ALLOC_H
#define ALLOC_H

#define SL_BLOCK_SIZE 4096
#include <stddef.h>
#include <assert.h>
#include <vector>
#include <set>
#include <algorithm>
#include <stdint.h>

#ifndef NDEBUG
#include <iostream>
#include <stdio.h>
#endif

namespace msgskip{
namespace misc{

template <typename alc>
class single_thread_arena{
	struct partial_buffer{
		char* buff_;
		size_t rest_;
		explicit partial_buffer(size_t size):buff_(NULL),rest_(size){}
		partial_buffer(char* buff,size_t rest)
			:buff_(reinterpret_cast<char*>(buff)),rest_(rest){}
		partial_buffer(partial_buffer&& p)
			:buff_(p.buff_),rest_(p.rest_)
		{}
		char* allocate(size_t required){
			assert(required <= rest_);
			char* const result = buff_;
			buff_ += required;
			rest_ -= required;
			return result;
		}
		size_t rest()const{return rest_;}
		bool operator<(const partial_buffer& rhs)const{
			return rest_ < rhs.rest_;
		}
		partial_buffer(const partial_buffer& p) = default;
		~partial_buffer() = default; // it has no duty of releasing buffer
#ifndef NDEBUG
		void dump()const{
			printf("PB: from %p, rest:%u\n",buff_, rest_);
		}
#endif
	};
	struct partial_buffer_comparator{
		bool operator()(size_t size,const partial_buffer& pb)const
		{return size < pb.rest_;}
		bool operator()(const partial_buffer& pb,size_t size)const
		{return pb.rest_ < size;}
		bool operator()(const partial_buffer& lhs, const partial_buffer& rhs)const
		{return lhs < rhs;}
	};
public:
	single_thread_arena()
		:alloc_ptr_(alctor.allocate(SL_BLOCK_SIZE)),rest_(SL_BLOCK_SIZE),
		 total_alloc_(SL_BLOCK_SIZE)
	{}
	char* allocate(size_t required){
		if(SL_BLOCK_SIZE <= required){
			char* result = alctor.allocate(required);
			total_alloc_+= required;
			memories_.push_back(result);
			return result;
		}

		// get from old arenas
		typename std::set<partial_buffer>::iterator target
			= buffer_set_.find(partial_buffer(required));
		if(target != buffer_set_.end()){
#ifndef NDEBUG
			printf("before:");target->dump();
#endif
			partial_buffer tmp(*target);
			char* result = tmp.allocate(required);
#ifndef NDEBUG
			printf("after:");target->dump();
#endif
			//buffer_.erase(target);
			buffer_set_.erase(target);
#ifndef NDEBUG
			printf("log:passed from partial %p\n", result);
#endif
			if(8 < target->rest()){
				//buffer_.insert(std::lower_bound(buffer_.begin(),buffer_.end(),tmp),tmp);
				buffer_set_.insert(tmp);
			}
			return result;
		}


		while(1){
			// get from active bucket
			if(required < rest_){
				char* const result = alloc_ptr_;
				alloc_ptr_+=required;
				rest_-= required;
#ifndef NDEBUG
				printf("log:passed from active %p\n", result);
#endif
				return result;
			}

			// get newpage
			buffer_set_.insert(partial_buffer(alloc_ptr_, rest_));
#ifndef NDEBUG
			printf("new partial:");
#endif
			alloc_ptr_ = alctor.allocate(SL_BLOCK_SIZE);
#ifndef NDEBUG
			printf("log: new allocate %p\n", alloc_ptr_);
#endif
			rest_ = SL_BLOCK_SIZE;
			total_alloc_ += SL_BLOCK_SIZE;
			memories_.push_back(alloc_ptr_); // for delete[] 
		}
	}
	size_t memory_usage()const{return total_alloc_;}
	~single_thread_arena(){
		for(std::vector<char*>::iterator iter=memories_.begin();
				iter != memories_.end();
				++iter){
			delete[] *iter;
		}
	}
	std::set<partial_buffer, partial_buffer_comparator> buffer_set_;
  alc alctor;
	char* alloc_ptr_;
	size_t rest_;
	std::vector<char*> memories_; // for delete[]

	// use for report
	uint64_t total_alloc_;

	single_thread_arena(const single_thread_arena&) = delete;
	single_thread_arena operator=(const single_thread_arena&) = delete;
};
}
}

#endif
