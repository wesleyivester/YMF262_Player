@echo off
make
echo Copying program to mbed...
del E:\*.bin
copy /B BUILD\*.bin E:\program-%random%.bin