#arm-none-eabi-as -mcpu=arm926ej-s startup.s -o startup.o
arm-none-eabi-as -mcpu=arm926ej-s test.s -o test.o
arm-none-eabi-ld -T test.ld test.o -o test.elf
arm-none-eabi-objcopy -O binary test.elf test.bin
qemu-system-arm -machine versatilepb -machine versatilepb -nographic -kernel test.bin
rm test.o test.elf
#rm test.bin
