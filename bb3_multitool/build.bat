cd otherapp_template && make clean
make && cd ..
make
python build.py
cp bb3.bin g:/bb3.bin
pause