all: quicksort quicksortconc
	@python rlist.py 10 > entradaP.txt
	@python rlist.py 1000000 > entradaG.txt
	@./quicksort 1 10 entradaP.txt
	@./quicksortconc 1 10 entradaP.txt 2
	@./quicksort 0 1000000 entradaG.txt
	@./quicksortconc 0 1000000 entradaG.txt 2
	@./quicksortconc 0 1000000 entradaG.txt 4
	@./quicksortconc 0 1000000 entradaG.txt 8
	@echo "##########################################################"
	
quicksort: quickSort.c
	@gcc quickSort.c -o quicksort
	
quicksortconc: quickSortConc.c
	@gcc quickSortConc.c -o quicksortconc -lpthread
