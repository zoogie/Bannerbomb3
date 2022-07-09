rm *.elf
rm *.dat
cd otherapp_template && make clean && make && cd ..
make
python TADmuffin.py 
pause