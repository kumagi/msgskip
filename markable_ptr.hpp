#ifndef MARKABLE_PTR_HPP
#define MARKABLE_PTR_HPP

#include <cassert>
#include <cstdint>
#include <cstdatomic>
namespace msgskip{
namespace misc{
/**
 * 1ビットの「マーク」情報を持つことができるポインタ型。
 */
template<typename T>
class markable_ptr;

template<>
class markable_ptr<void>{

private:
	typedef std::atomic<void> markable_ptr_t;
  std::atomic<void*> mptr_;
public:
  markable_ptr(void* mp) : mptr_(mp) {}

  inline void* load_relaxed(){return mptr_.load(std::memory_order_relaxed);	}
	inline const void* load_relaxed()const{return mptr_.load(std::memory_order_relaxed);	}
  inline void* load_acquire(){return mptr_.load(std::memory_order_acquire);	}
	inline const void* load_acquire()const {return mptr_.load(std::memory_order_acquire);	}
  inline void store_relaxed(void* p){mptr_.store(p, std::memory_order_relaxed);	}
	inline void store_release(void* p){mptr_.store(p, std::memory_order_release);	}

  bool compare_and_set(void* expected, void* newone){
		return mptr_.compare_exchange_strong(expected, newone, std::memory_order_seq_cst, std::memory_order_seq_cst);
	}

public:
  markable_ptr() = default;
  markable_ptr(void* p, bool marked = false)
		: mptr_(reinterpret_cast<void*>(
							marked ?
							reinterpret_cast<std::uintptr_t>(p) | 1 :
							reinterpret_cast<std::uintptr_t>(p)) ) { }
	bool try_mark(){
		const std::uintptr_t old_ptr = reinterpret_cast<std::uintptr_t>(load_acquire());
		if((old_ptr&1) == 1) { return false; }
		const std::uintptr_t new_ptr = old_ptr | 1;
		void* old_ = reinterpret_cast<void*>(old_ptr);
		return mptr_.compare_exchange_strong(
			reinterpret_cast<void*>(old_),
			reinterpret_cast<void*>(new_ptr),
			std::memory_order_seq_cst,
			std::memory_order_seq_cst);
	}

  bool is_marked() const {
    return (reinterpret_cast<std::uintptr_t>(load_acquire()) & 1) != 0;
  }

  void* get() const { // マークされてないポインタを獲得する
		return reinterpret_cast<void*>
			(reinterpret_cast<std::uintptr_t>(load_relaxed()) & ~1LLU);
  }

	bool operator==(const void* rhs)const{
		return mptr_.load(std::memory_order_relaxed) == rhs;
	}
	bool operator!=(const void* rhs)const{
		return !this->operator==(rhs);
	}
  bool operator==(const markable_ptr& rhs) const {
    return mptr_ == rhs.mptr_;
  }
  bool operator!=(const markable_ptr& rhs)const{
		return !this->operator==(rhs);
	}
};

typedef markable_ptr<void> m_ptr;
// thin template
template<typename T>
class markable_ptr {
private:
	typedef markable_ptr<T> markable_ptr_t;
  m_ptr mptr_;
public:
  markable_ptr(T* mp) : mptr_(mp) {}
  inline T* load_relaxed(){return reinterpret_cast<T*>(mptr_.load_relaxed());}
	inline const T* load_relaxed()const{
		return reinterpret_cast<const T*>(mptr_.load_relaxed());
	}
	inline T* load_acquire(){return reinterpret_cast<T*>(mptr_.load_acquire());}
	inline const T* load_acquire()const{
		return reinterpret_cast<T*>(mptr_.load_acquire());
	}
  inline void store_relaxed(T* p){mptr_.store_relaxed(reinterpret_cast<void*>(p));	}
	inline void store_release(T* p){mptr_.store_release(reinterpret_cast<void*>(p));	}

  bool compare_and_set(T* expected, T* newone){
		return mptr_.compare_and_set(
			reinterpret_cast<void*>(expected),
			reinterpret_cast<void*>(newone));
	}
  bool compare_and_set(const markable_ptr_t expected, T* const newone){
		return mptr_.compare_and_set(
			reinterpret_cast<void*>(expected.load_relaxed()),
			reinterpret_cast<void*>(newone));
	}
  bool compare_and_set(const T* const expected, markable_ptr_t newone){
		return mptr_.compare_and_set(
			reinterpret_cast<void*>(expected),
			reinterpret_cast<void*>(newone.load_relaxed()));
	}
  bool compare_and_set(const markable_ptr_t expected, markable_ptr_t newone){
		return mptr_.compare_and_set(
			reinterpret_cast<void*>(expected.load_relaxed()),
			reinterpret_cast<void*>(newone.load_relaxed()));
	}
public:
  markable_ptr() = default;
  markable_ptr(T* p, bool marked = false)
		: mptr_(marked ?
						reinterpret_cast<std::uintptr_t>(p) | 1 :
						reinterpret_cast<std::uintptr_t>(p) ) { }
	bool try_mark(){ return mptr_.try_mark();	}
  bool is_marked() const {return mptr_.is_marked(); }
  T* get() const { return reinterpret_cast<T*>(mptr_.get());}
	bool operator==(const T* rhs)const{
		return load_acquire() == rhs.load_acquire();
	}
	bool operator!=(const T* rhs)const{return !this->operator==(rhs);	}
  bool operator==(const markable_ptr& rhs) const {return mptr_ == rhs.mptr_;}
  bool operator!=(const markable_ptr& rhs)const{return !this->operator==(rhs);}
};

}
}
#endif
