[ "$1" == "clean" ] && rm -rf _benchmarkdb && rm -rf *.bin && rm -rf *.exe && rm -rf *.obj && rm -rf *.o && exit

echo "Build crossdb-bench.bin"
gcc -o crossdb-bench.bin crossdb-bench.c -I.. -lcrossdb -O3 -Wall -pthread -ldl
echo "Build sqlite-bench.bin"
gcc -o sqlite-bench.bin  sqlite-bench.c -lsqlite3 -O3 -Wall -pthread
echo "Build lmdb-bench.bin"
gcc -o lmdb-bench.bin    lmdb-bench.c -llmdb -O3 -Wall -pthread
echo "Build stlmap-bench.bin"
g++ -o stlmap-bench.bin  stlmap-bench.cpp -O3 -Wall -pthread
echo "Build stlhmap-bench.bin"
g++ -o stlhmap-bench.bin stlhmap-bench.cpp -O3 -Wall -pthread
