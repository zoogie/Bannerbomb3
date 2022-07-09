#include <cstring>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "crypto.h"
#include "tadpole.h"
#include "types.h"

void getSection(u8 *dsiware_pointer, u32 section_size, u8 *key, u8 *output) {
        decryptAES(dsiware_pointer, section_size, key, (dsiware_pointer + section_size + 0x10), output);
}

void placeSection(u8 *dsiware_pointer, u8 *section, u32 section_size, u8 *key, u8 *key_cmac) {
        u8 allzero[0x10]= {0};

        encryptAES(section, section_size, key, allzero, dsiware_pointer);

        u8 section_hash[0x20] = {0};
        calculateSha256(section, section_size, section_hash);
        u8 section_cmac[0x20] = {0};
        calculateCMAC(section_hash, 0x20, key_cmac, section_cmac);

        memcpy((dsiware_pointer + section_size), section_cmac, 0x10);
        memset((dsiware_pointer + section_size + 0x10), 0, 0x10);
}