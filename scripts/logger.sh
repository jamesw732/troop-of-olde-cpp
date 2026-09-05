./logger.exe server-in.bin > server-in.log
./logger.exe server-out.bin > server-out.log

for file in client-{in,out}-*.bin; do
    ./logger.exe "$file" > "${file%.bin}.log"
done
