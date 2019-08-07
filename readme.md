# Bannerbomb3

## Intro

This is a POC for a new System Settings userland exploit. 
It uses ROP and ARM execution to dump DS Internet (and possibly others) from System Settings using a custom crafted dsiware export.
This is useful primarily as an enhancement for "Fredminer" variant of seedminer to obtain free cfw on 3ds.

Among other things, it brings free cfw to more regions*, and removes the possibility of Nintendo pulling certain games like Steel Diver from the eshop to thwart homebrew efforts.

*(except China - iQue System Settings cannot access dsiware)

## Directions 

Directions are provided in the Release archive.

Optionally, here's an online service for non-windows users (also has Taiwan support):
https://jenkins.nelthorya.net/job/DSIHaxInjector%20v2/

Using bannerbom3 in conjunction with decent homebrew guides is probably the best strategy for most users, though.
https://3ds.hacks.guide/seedminer (like this one)

## Hbmenu? 

I've been able to get otherapp.bin booting by using 3ds_ropkit and a loader ROP chain. However, shortly after the bottom screen turns yellow, the 3ds just reboots to home menu.
Debugging this, it seems like otherapp is crashing on _aptExit() around here:
https://github.com/smealum/ninjhax2.x/blob/o3ds_newpayloads/cn_secondary_payload/source/main.c#L629

It's really alright though. Fredminer gets you a more stable 3dsx homebrew environment anyway, so this isn't really a high priority issue right now (still would be cool to see hbmenu booting I admit).

## Exploit 

Basically put, this overflows the banner title strings in DSiWare exports (TADs) when you view them in System Settings, and smashes the stack leading to ROP control for the attacker.
You do need the movable.sed to encrypt a payload TAD, but that's easy enough to do nowadays. Movable.sed bruteforcing now only takes about a minute and free online services can do it for you. Over 350,000 people have done it so it can't be that hard :p

More exploit details on 3dbrew:
https://www.3dbrew.org/wiki/3DS_Userland_Flaws#System_applications
... and in the comments inside rop_payload/rop_payload.s, of course.

## Q&A 

Q: What's with the 3 in Bannerbomb3?
A: It's a tribute to the Wii scene, they did 1 & 2. I love old homebrew scenes.

Q: Why TADmuffin?
A: Muffin sounded funny so I went with that. Just needed to be different from TADpole.

Q: Will this work on the DSi since it has DSiWare exports too?
A: The flaw is definitely there as well, but I've been unsuccessful exploiting it on hardware (I can get code exe on no$gba though). Moot because of Memory Pit anyhow ;)

Q: Is this your first 3ds userland exploit?
A: Yes. Feels good man.

## Thanks 

- Yellows8 for 3ds ropkit
- All the people on #3dsdev, reading my backlog (Ctrl-F "pivot") provided a wealth of good info on the art of stack pivoting.
- Nintendo Homebrew Discord for maintaining online tools/guides and helping all the seed/frog/fredminer users. I hope this sploit makes your jobs a little easier.
- Jhynjhiruu for testing
- Smea for regionFour, which I base the arm part of code_payload on.
- Wintermute for ROPinstaller, for the gspwn codeload ROP (Bootstrap.S) that I used in code_payload.