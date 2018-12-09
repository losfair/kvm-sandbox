all: loader
	$(CXX) -Wall -O2 -o kvm_start -std=c++17 kvm_start.cpp vm.cpp gdt.cpp error.cpp -lpthread

loader:
	make -C loader

.PHONY: loader
