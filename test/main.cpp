#include <iostream>
#include <vector>
#include <list>
#include "../lego/heap_allocator.h"
#include "../lego/null_allocator.h"
#include "../lego/stl_adapter.h"
#include "../lego/fallback_allocator.h"
#include "../lego/log_allocator.h"
#include "../lego/stack_allocator.h"
using namespace std;
using namespace lego;


void TestSTLOnVector() {
	using Allocator = STLAdapter<int, HeapAllocator>;
	cout << "==== TestSTLOnVector()" << endl;
	vector<int, Allocator> vec1;
	for (int i = 0; i < 10; ++i) {
		vec1.push_back(i);
	}
	bool green = true;
	int i = 0;
	for (auto&& element : vec1) {
		if (element != i++) {
			green = false;
			break;
		}
	}
	cout << "Testing..." << (green ? "YES!" : "NO!") << endl;
	cout << endl;
}
void TestSTLOnList() {
	using Allocator = STLAdapter<int, HeapAllocator>;
	cout << "==== TestSTLOnList()" << endl;
	list<int, Allocator> list1;
	for (int i = 0; i < 10; ++i) {
		list1.push_back(i);
	}

	bool green = true;
	int i = 0;
	for (auto&& element : list1) {
		if (element != i++) {
			green = false;
			break;
		}
	}
	cout << "Testing..." << (green ? "YES!" : "NO!") << endl;
	cout << endl;
}

void TestFallbackLocalHeap() {
	cout << "=== Testing Fallback/Local/Heap/Stack allocators" << endl;
	struct StackLogStrategy {
		void printAllocate(Blk blk) {
			std::cout << "Stack Allocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
		void printDeallocate(Blk blk) {
			std::cout << "Stack Deallocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
	};


	struct MallocLogStrategy {
		void printAllocate(Blk blk) {
			std::cout << "Malloc Allocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
		void printDeallocate(Blk blk) {
			std::cout << "Malloc Deallocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
	};

	using Allocator = FallbackAllocator<
		LogAllocator<LocalStackAllocator<50>, StackLogStrategy>,
		LogAllocator<HeapAllocator, MallocLogStrategy>>;

	Allocator allocator;
	// Stack allocator of 50 bytes should only be able to take 50 / (4 + 1) = 10 4-byte allocations
	// +1 is due to the bookkeeping required per allocator

	for (int i = 0; i < 20; ++i) {
		if (i == 10)
			cout << "Fallback should happen here!" << endl;
		allocator.allocate(4, 4);

	}

	cout << endl;
}

void TestStackAllocator() {
	cout << "=== Testing Fallback/Local/Heap/Stack allocators" << endl;
	// Stack allocator of 100 bytes should only be able to take 100 / (4 + 1) = 20 4-byte allocations
	// +1 is due to the bookkeeping required per allocator
	using Allocator = HeapStackAllocator<100>;
	Allocator allocator;

	auto startBlk = allocator.allocate(4, 4); // store the memory address of first item's allocation
	allocator.deallocate(startBlk);

	vector<Blk> blkList;
	for (int i = 0; i < 20; ++i) {
		blkList.push_back(allocator.allocate(4, 4));
	}
	auto expectNullBlk = allocator.allocate(4, 4);
	cout << "Testing allocation integrity..." << (!expectNullBlk ? "YES" : "NO") << endl;

	// deallocate test
	for (int i = 19; i >= 0; --i) {
		allocator.deallocate(blkList[i]);
	}
	auto tmpBlk = allocator.allocate(4, 4);
	cout << "Testing deallocation integrity..." << (tmpBlk == startBlk ? "YES" : "NO") << endl;
	allocator.deallocate(tmpBlk);

	//deallocate all test
	for (int i = 0; i < 20; ++i) {
		blkList[i] = allocator.allocate(4, 4);
	}
	allocator.deallocateAll();
	tmpBlk = allocator.allocate(4, 4);
	allocator.deallocate(tmpBlk);

	cout << "Testing deallocateAll integrity..." << (tmpBlk == startBlk ? "YES" : "NO") << endl;

	cout << endl;
}

int main() {
	TestSTLOnVector();
	TestSTLOnList();
	TestFallbackLocalHeap();

	TestStackAllocator();
}