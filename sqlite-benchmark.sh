cpu=$1
[ "$1" == "" ] && cpu=-1
loop="1 2 3"

echo "On-Disk Benchmark Test"
./sqlite-bench.bin -H -r 0
for i in $loop; do ./sqlite-bench.bin -s d -i 1k   -q 1m  -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s d -i 10k  -q 1m  -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s d -i 100k -q 1m  -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s d -i 1m   -q 1m  -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s d -i 10m  -q 1m  -u 1m  -Q -H -c $cpu; done
# this test is very very very slow
#for i in $loop; do ./sqlite-bench.bin -s d -i 100m -q 1m  -u 1m  -Q -H -c $cpu; done

echo "On-RamDisk Benchmark Test"
./sqlite-bench.bin -H -r 0
for i in $loop; do ./sqlite-bench.bin -s r -i 1k   -q 1m -u 500k  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s r -i 10k  -q 1m -u 500k  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s r -i 100k -q 1m -u 300k  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s r -i 1m   -q 500k -u 250k -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s r -i 10m  -q 500k -u 250k -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s r -i 100m -q 500k -u 250k -Q -H -c $cpu; done

echo "In-Memory Benchmark Test"
./sqlite-bench.bin -H -r 0
for i in $loop; do ./sqlite-bench.bin -s m -i 1k   -q 2m -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s m -i 10k  -q 2m -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s m -i 100k -q 2m -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s m -i 1m   -q 1m -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s m -i 10m  -q 1m -u 1m  -Q -H -c $cpu; done
for i in $loop; do ./sqlite-bench.bin -s m -i 100m -q 1m -u 1m  -Q -H -c $cpu; done
