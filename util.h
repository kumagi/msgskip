#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>
#include <msgpack.hpp>
#include <assert.h>
#include <stdio.h>

namespace msgskip{
namespace misc{
template <typename T>
size_t estimate_packed_size(const T& t){
	assert(&t != NULL);
	printf("ptr:%p\n",&t);
	msgpack::sbuffer sb;
	msgpack::pack(sb,t);
	return sb.size();
}
}// misc
}// msgskip
#endif
