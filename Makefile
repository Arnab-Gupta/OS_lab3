mmu: src/mmu.cpp src/pager.h src/vma.h src/process.h
	g++ -std=c++1y src/mmu.cpp -o mmu
clean:
	rm -f mmu *~