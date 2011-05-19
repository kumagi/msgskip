#include <gtest/gtest.h>
#include "alloc.h"
#include <iostream>
#include <stdlib.h>
#include <random>

TEST(alloc,construct){
	msgskip::misc::single_thread_arena<std::allocator<char> > a;
}
TEST(alloc,and_use){
	msgskip::misc::single_thread_arena<std::allocator<char> > a;
	char* ptr = a.allocate(523);
#ifndef NDEBUG
	std::cout << "memory usage:" << a.memory_usage() << std::endl;
#endif
	for(int i=0;i<523;i++){
		ptr[i] = (i*31)&255;
	}
}

TEST(alloc,many){
	msgskip::misc::single_thread_arena<std::allocator<char> > a;
	for(int j=0;j<5048;++j){
		char* ptr = a.allocate(523);
		for(int i=0;i<523;i++){
			ptr[i] = (i*31)&255;
		}
#ifndef NDEBUG
		std::cout << "memory usage:" << a.memory_usage() << std::endl;
#endif
	}
}

TEST(alloc,random){
	msgskip::misc::single_thread_arena<std::allocator<char> > a;
	for(int j=0;j<1024;++j){
		const size_t rand_size = rand() % 8196;
		
#ifndef NDEBUG
		std::cout << "begin allocate:" << rand_size << std::endl;
#endif
		char* ptr = a.allocate(rand_size);
		for(size_t i=0;i<rand_size;i++){
			ptr[i] = (i*31)&255;
		}
#ifndef NDEBUG
		std::cout << "memory usage:" << a.memory_usage() << std::endl;
#endif
	}
}
