cd otherapp_template && make clean
make && cd ..
make
python build.py
cp bb3.bin f:/bb3.bin
pause