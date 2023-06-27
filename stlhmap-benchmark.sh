cpu=$1
[ "$1" == "" ] && cpu=-1
loop="1 2 3"

echo "In-Memory Benchmark Test"
./stlhmap-bench.bin -H -r 0
for i in $loop; do ./stlhmap-bench.bin -s m -i 1k   -q 60m -u 60m -Q -H -c $cpu; done
for i in $loop; do ./stlhmap-bench.bin -s m -i 10k  -q 60m -u 60m -Q -H -c $cpu; done
for i in $loop; do ./stlhmap-bench.bin -s m -i 100k -q 30m -u 40m -Q -H -c $cpu; done
for i in $loop; do ./stlhmap-bench.bin -s m -i 1m   -q 10m -u 10m -Q -H -c $cpu; done
for i in $loop; do ./stlhmap-bench.bin -s m -i 10m  -q 10m -u 10m -Q -H -c $cpu; done
for i in $loop; do ./stlhmap-bench.bin -s m -i 100m -q 10m -u 10m -Q -H -c $cpu; done
