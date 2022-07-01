Bannerbomb3 bb3.bin script (USA, EUR, JPN, KOR, old/new3ds, latest version of mset) (script is python3 only)

This tadmuffin variant simply reads bb3.bin off of the sdmc root, loads it to mset BSS address 0x00682000, and pivots to it. Size up to 0x80200, which is probably overkill but whatever.
The purpose of this is to be as modular as possible, and stable. 
I would like to think of this as the default version of bannerbomb3 (or really should have been had I some forsight).

FREE BONUS: Demo bb3.bin ropchain and source included.
Check inside the demo folder, place the bb3.bin file on the sd root. Run exploit (as a dsiware exploit built from this directory's TADmuffin.py).
When you tire of the amazing visual display, hold A + B to reboot.