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
class markable_ptr {
private:
	typedef markable_ptr<T> markable_ptr_t;
  std::atomic<T*> mptr_;
public:
  markable_ptr(T* mp) : mptr_(mp) {}

  T* load_relaxed(){return mptr_.load(std::memory_order_relaxed);	}
  T* load_acquire(){return mptr_.load(std::memory_order_acquire);	}
  void store_relaxed(T* const p){return mptr_.store(p, std::memory_order_acquire);	}
	void store_relaxed(const T* const p){return mptr_.store(p, std::memory_order_acquire);	}
	void store_release(T* const p){return mptr_.store(p, std::memory_order_acquire);	}
	void store_release(const T* const p){return mptr_.store(p, std::memory_order_acquire);	}

  bool compare_and_set(const T* const expected, T* const newone){
		return mptr_.compare_exchange_strong(expected, newone);
	}
  bool compare_and_set(const markable_ptr_t expected, T* const newone){
		return mptr_.compare_exchange_strong(expected.load_relaxed(), newone);
	}
  bool compare_and_set(const T* const expected, markable_ptr_t newone){
		return mptr_.compare_exchange_strong(expected, newone.load_relaxed());
	}
  bool compare_and_set(const markable_ptr_t expected, markable_ptr_t newone){
		return mptr_.compare_exchange_strong(expected.load_relaxed(), newone.load_relaxed());
	}

 public:
  markable_ptr() = default;
  markable_ptr(T* p, bool marked = false)
		: mptr_(marked ?
						reinterpret_cast<std::uintptr_t>(p) | 1 :
						reinterpret_cast<std::uintptr_t>(p) ) { }

	
  markable_ptr_t mark() const {
    return markable_ptr(mptr_ | 1);
  }

	/**
	 * atomicにマーキングする
	 */
	bool try_mark(){
		const std::uintptr_t old_ptr = reinterpret_cast<std::uintptr_t>(mptr_.load_acquire());
		if(old_ptr&1 == 1) { return false; }
		const std::uintptr_t new_ptr = old_ptr | 1;
		return mptr_.compare_exchange_strong(old_ptr,new_ptr);
	}

  /**
   * このmarkable_ptrとポインタ値が同じで、マーク無しのmarkable_ptrを返す。
   * このmarkable_ptr自体のマークの有無はどちらでもよい。
   */
  markable_ptr_t to_unmarked() const {
    return markable_ptr(mptr_ & ~1);
  }

  /**
   * このmarkable_ptrがマーク有りならばtrueを返す。
   */
  bool is_marked() const {
    return (mptr_ & 1) != 0;
  }

  T* get() const {
		std::uintptr_t old_ptr = mptr_;
		if(old_ptr&1) old_ptr ^= 1;
    return reinterpret_cast<T*>(old_ptr);
  }

	bool operator==(const T* rhs)const{
		return mptr_.load(std::memory_order_relaxed) == rhs;
	}
	bool operator!=(const T* rhs)const{
		return !this->operator==(rhs);
	}
  bool operator==(const markable_ptr& rhs) const {
    return mptr_ == rhs.mptr_;
  }
  bool operator!=(const markable_ptr& rhs)const{
		return !this->operator==(rhs);
	}
};

}
}
#endif
