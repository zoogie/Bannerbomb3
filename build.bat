cd rop_payload && make clean && make
cp rop_payload.bin ../TADmuffin/data/
cd ..

cd TADmuffin
bin2c -n payload -o include/payload.h data/rop_payload.bin
make clean && make

pause