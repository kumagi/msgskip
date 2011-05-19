#include <iostream>
#include "alloc.h"

int main(){
	msgskip::misc::single_thread_arena<std::allocator<char> > a;
	size_t required_memory = 0;
	const int repeat = 1024 * 512;
	for(int j=0;j<repeat ;++j){
		const size_t rand_size = rand() % 8196;
		required_memory += rand_size;
		char* ptr = a.allocate(rand_size);
		for(size_t i=0;i<rand_size;i++){
			//ptr[i] = (i*31)&255;
		}
		if((j&31) == 0){
			printf("\r %5.2f %%", (double)j * 100/repeat);
		}
	}
	printf("\r");
	std::cout << "really needed:" << required_memory << std::endl;
	std::cout << "memory usage:" << a.memory_usage() << std::endl;
	std::cout << "efficiency:" << (double)required_memory * 100 / a.memory_usage() << "%" << std::endl;
}
