all: loader
	python3 ./syscall/gen.py ./syscall/table > syscall_table.generated.hpp
	$(CXX) -Wall -O2 -o kvm_start -std=c++17 kvm_start.cpp vm.cpp gdt.cpp error.cpp -lpthread

loader:
	make -C loader

clean:
	rm kvm_start || true
	make -C loader clean

.PHONY: loader
