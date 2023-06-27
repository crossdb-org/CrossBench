cpu=$1
[ "$1" == "" ] && cpu=-1
loop="1 2 3"

echo "In-Memory Benchmark Test"
./stlmap-bench.bin -H -r 0
for i in $loop; do ./stlmap-bench.bin -s m -i 1k   -q 40m -u 40m -Q -H -c $cpu; done
for i in $loop; do ./stlmap-bench.bin -s m -i 10k  -q 20m -u 20m -Q -H -c $cpu; done
for i in $loop; do ./stlmap-bench.bin -s m -i 100k -q 10m -u 10m -Q -H -c $cpu; done
for i in $loop; do ./stlmap-bench.bin -s m -i 1m   -q 5m  -u 5m  -Q -H -c $cpu; done
for i in $loop; do ./stlmap-bench.bin -s m -i 10m  -q 2m  -u 2m  -Q -H -c $cpu; done
for i in $loop; do ./stlmap-bench.bin -s m -i 100m -q 1m  -u 1m  -Q -H -c $cpu; done
