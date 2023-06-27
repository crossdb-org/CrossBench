cpu=$1
[ "$1" == "" ] && cpu=-1
loop="1 2 3"

echo "On-Disk Benchmark Test"
./lmdb-bench.bin -H -r 0
for i in $loop; do ./lmdb-bench.bin -s d -i 1k   -q 10m -u 5m -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s d -i 10k  -q 10m -u 5m -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s d -i 100k -q 10m -u 5m -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s d -i 1m   -q 5m  -u 5m -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s d -i 10m  -q 2m  -u 2m -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s d -i 100m -q 1m  -u 1m -Q -H -c $cpu; done

echo "On-RamDisk Benchmark Test"
./lmdb-bench.bin -H -r 0
for i in $loop; do ./lmdb-bench.bin -s r -i 1k   -q 10m -u 5m  -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s r -i 10k  -q 10m -u 5m  -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s r -i 100k -q 10m -u 5m  -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s r -i 1m   -q 5m  -u 5m  -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s r -i 10m  -q 2m  -u 2m  -Q -H -c $cpu; done
for i in $loop; do ./lmdb-bench.bin -s r -i 100m -q 1m  -u 1m  -Q -H -c $cpu; done
