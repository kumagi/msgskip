#include <iostream>
#include <vector>

int main(){
	size_t required_memory = 0;
	const int repeat = 1024 * 512;
	std::vector<char*> memories;
	for(int j=0;j<repeat ;++j){
		const size_t rand_size = rand() % 8196;
		required_memory += rand_size;
		memories.push_back(new char[rand_size]);
		if((j&31) == 0){
			printf("\r %5.2f %%", (double)j * 100/repeat);
		}
	}
	printf("\r");
	for(int j=0;j<repeat;++j){
		delete[] memories[j];
	}
}
