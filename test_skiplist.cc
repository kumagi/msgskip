#include <gtest/gtest.h>
#include <stdlib.h>
#include "skiplist.h"
TEST(a,2){
	msgskip::skiplist<int,int> sl;
}

TEST(skiplist,insert){
	msgskip::skiplist<int,int> sl;
	sl.insert(32,4);
}

TEST(skiplist,contains){
	msgskip::skiplist<int,int> sl;
	sl.insert(32,4);
	//sl.dump();
	EXPECT_EQ(sl.contains(32),true);
}

TEST(skiplist,three_insert){
	msgskip::skiplist<int,int> sl;
	sl.insert(121,2);
	sl.insert(12,1);
	sl.insert(15,1);
	EXPECT_TRUE(sl.contains(121));
	EXPECT_TRUE(sl.contains(12));
	EXPECT_TRUE(sl.contains(15));
	// sl.dump();
}

TEST(skiplist,many_insert){
	msgskip::skiplist<int,int> sl;
	srand(0);
	int cnt = 0;
	for(int i=0;i<256;++i){
		if(sl.insert(rand() & 511,i)){
			cnt++;
		}
	}
	//printf("items:%d\n",cnt);
	
	srand(0);
	for(int i=0;i<256;++i){
		const int target = rand() & 511;
		//printf("search %d\n", target);
		EXPECT_TRUE(sl.contains(target));
	}
	// sl.dump();
}





