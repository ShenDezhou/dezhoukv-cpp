all:	unittest1

unittest1: unittest1.o
	g++ $^ -o $@ -lboost_unit_test_framework

check:	all
	./unittest1

