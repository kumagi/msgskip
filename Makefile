CC=g++
LD=-lpthread -lmsgpack
OPTS=-g -O0 -std=c++0x
GTEST_INC= -I$(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_DIR=/opt/google/gtest-1.6.0
WARNS= -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=4 -Wcast-qual -Wcast-align \
	-Wwrite-strings -Wfloat-equal -Wpointer-arith -Wswitch-enum
NOTIFY=&& notify-send Test success! -i ~/themes/ok_icon.png || notify-send Test failed... -i ~/themes/ng_icon.png


target:test_skiplist
target:test_alloc
target:alloc_efficiency
target:alloc_compare
test_skiplist:test_skiplist.o libgtest.a
	$(CC) $^ $(OPTS) $(LD) $(WARNS) -o $@
	./$@ $(NOTIFY)
test_skiplist.o:test_skiplist.cc skiplist.h markable_ptr.hpp
	$(CC) $< $(GTEST_INC) $(OPTS) $(WARNS) -o $@ -c
alloc_efficiency:alloc_efficiency.cc alloc.h
	$(CC) $< $(OPTS) $(WARNS) -o $@
alloc_compare:alloc_compare.cc alloc.h
	$(CC) $< $(OPTS) $(WARNS) -o $@

test_alloc:test_alloc.o libgtest.a
	$(CC) $^ $(OPTS) $(LD) $(WARNS) -o $@
	./$@ $(NOTIFY)
test_alloc.o:test_alloc.cc alloc.h	
	$(CC) $< $(GTEST_INC) $(OPTS) $(WARNS) -o $@ -c

# gtest
libgtest.a:
	$(CXX) ${GTEST_INC} -c ${GTEST_DIR}/src/gtest-all.cc -o gtest-all.o
	$(CXX) ${GTEST_INC} -c ${GTEST_DIR}/src/gtest_main.cc -o gtest_main.o
	ar -rv libgtest.a gtest-all.o gtest_main.o

gtest_main.o:
	$(CXX) $(GTEST_INC) -c $(OPTS) $(GTEST_DIR)/src/gtest_main.cc -o $@
gtest-all.o:
	$(CXX) $(GTEST_INC) -c $(OPTS) $(GTEST_DIR)/src/gtest-all.cc -o $@
gtest_main.a:gtest-all.o gtest_main.o
	ar -r $@ $^

.cc.o:
	$(CXX) $(GTEST_INC) $(OPTS) $< -o $@

clean:
	rm -f *.o
	rm -f *~
