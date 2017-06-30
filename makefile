CC = g++
CPPFLAGS =  -fopenmp -ansi -pedantic -O3 -std=c++0x

objects = Apsp.o Tools.o testutils.o Ctree.o 

default: $(objects)
	$(CC) $(CPPFLAGS) -o Apsp $(objects)
	$(CC) gen_test.cpp -o gen
	$(CC) converter_fasta_to_plain.cpp -o converter

clean:
	rm -f core *.exe *.o *~ Apsp test.* 



