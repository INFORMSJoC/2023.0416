# test.cpp tabu_search.cpp basic.h
tar = ilrp
obj = crossover.o Individual.o initialsol.o LS.o mutation.o popManager.o read_data.o RepairProcedure.o solution.o main.o
deps = crossover.h Individual.h initialsol.h LS.h mutation.h popManager.h read_data.h RepairProcedure.h solution.h commandline.h basic.h
cc = g++
RMRF :=rm -rf

$(tar): $(obj)      # transfer to exe files
	$(cc) $^ -o $@
#test: tabu_search.o test.o
#	g++ tabu_search.o test.o -o test
%.o: %.c $(deps)    # head files
	$(cc) -c %.c -o %.o   # transfer a file
#test.o: test.cpp
#	g++ -c test.cpp -o test.o
#tabu_search.o: tabu_search.cpp
#	g++ -c tabu_search.cpp -o tabu_search.o

.PHONY:
clearall:
	$(RMRF) $(obj) $(tar)
clear:
	$(RMRF) $(obj) 
