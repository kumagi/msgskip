#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>
#include <cstddef>
#include <msgpack.hpp>
#include <assert.h>
#include <stdio.h>

namespace msgskip{
namespace misc{
template <typename T>
std::size_t estimate_packed_size(const T& t){
	assert(&t != NULL);
	// printf("ptr:%p\n",&t);
	msgpack::sbuffer sb;
	msgpack::pack(sb,t);
	return sb.size();
}
template <typename type>
std::size_t pack_it(void* b, const type* const v){
	if(v){
		msgpack::sbuffer sb;
		msgpack::pack(sb, *v);
		memcpy(b, sb.data(), sb.size());
		return sb.size();
	}else{
		msgpack::sbuffer sb;
		msgpack::pack(sb, NULL);
		memcpy(b, sb.data(), sb.size());
		return sb.size();
	}
}
}// misc
}// msgskip
#endif
