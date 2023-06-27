cpu=$1
[ "$1" == "" ] && cpu=-1
loop="1 2 3"

echo "On-Disk Benchmark Test"
./crossdb-bench.bin -H -r 0
for i in $loop; do ./crossdb-bench.bin -s d -i 1k   -q 30m -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s d -i 10k  -q 30m -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s d -i 100k -q 20m -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s d -i 1m   -q 10m -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s d -i 10m  -q 5m  -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s d -i 100m -q 5m  -u 1m  -Q -H -c $cpu; done

echo "On-RamDisk Benchmark Test"
./crossdb-bench.bin -H -r 0
for i in $loop; do ./crossdb-bench.bin -s r -i 1k   -q 40m -u 30m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s r -i 10k  -q 40m -u 30m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s r -i 100k -q 40m -u 30m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s r -i 1m   -q 10m -u 10m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s r -i 10m  -q 10m -u 10m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s r -i 100m -q 10m -u 10m -Q -H -c $cpu; done

echo "In-Memory Benchmark Test"
./crossdb-bench.bin -H -r 0
for i in $loop; do ./crossdb-bench.bin -s m -i 1k   -q 40m -u 30m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s m -i 10k  -q 40m -u 30m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s m -i 100k -q 40m -u 30m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s m -i 1m   -q 10m -u 10m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s m -i 10m  -q 10m -u 10m -Q -H -c $cpu; done
for i in $loop; do ./crossdb-bench.bin -s m -i 100m -q 10m -u 10m -Q -H -c $cpu; done
