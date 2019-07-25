#include "types.h"
#include "aes-cbc-cmac.h"
#include "crypto.h"
#include "sha256.h"


void encryptAES(u8 *plaintext, u32 size, u8 *key, u8 *iv, u8 *output) {
	AES_CBC_ENC(iv, key, plaintext, size, output, size);
}

void decryptAES(u8 *ciphertext, u32 size, u8 *key, u8 *iv, u8 *output) {
	AES_CBC_DEC(iv, key, ciphertext, size, output, size);
}

void calculateCMAC(u8 *input, u32 size, u8 *key, u8 *output) {
	AES_CMAC(key, input, size, output);
}

void calculateSha256(u8 *input, u32 size, u8 *output) {
	//FSUSER_UpdateSha256Context(input, size, output);
	SHA256_CTX *ctx = (SHA256_CTX*)malloc(sizeof(SHA256_CTX));
	sha256_init(ctx);
    sha256_update(ctx, input, size);
    sha256_final(ctx, output);
	free(ctx);
}

// Full credit goes to https://github.com/luigoalma/3ds_keyscrambler/blob/master/src/UnScrambler.c#L50
void keyScrambler(u8 *Y, bool cmacYN, u8 *normal_key) {
	u8 C[0x10] = {0x1F, 0xF9, 0xE9, 0xAA, 0xC5, 0xFE, 0x04, 0x08, 0x02, 0x45, 0x91, 0xDC, 0x5D, 0x52, 0x76, 0x8A};
	u8 X_normal[0x10] = {0x6F, 0xBB, 0x01, 0xF8, 0x72, 0xCA, 0xF9, 0xC0, 0x18, 0x34, 0xEE, 0xC0, 0x40, 0x65, 0xEE, 0x53};
	u8 X_cmac[0x10] = {0xB5, 0x29, 0x22, 0x1C, 0xDD, 0xB5, 0xDB, 0x5A, 0x1B, 0xF2, 0x6E, 0xFF, 0x20, 0x41, 0xE8, 0x75};
	u8 *X = (cmacYN) ? X_cmac : X_normal;

	int i;
	u8 shifted_X[0x10];
	for(i=0;i<16;i++){
		//The joke is already dead, but I keep doing it. So, more type casts and masks.
		if(i!=15) shifted_X[i] = (u8)((((u8)X[i] << (u8)2)&0xFC) + (((u8)X[i+1] >> (u8)6)&0x03))&0xff;
		else shifted_X[i] = (u8)((((u8)X[i] << (u8)2)&0xFC) + (((u8)X[0] >> (u8)6)&0x03))&0xff;
	}
	u8 shifted_X_xor_Y[0x10];
	for(i=0;i<16;i++){
		//Do I need to say anything more about the type casts?
		shifted_X_xor_Y[i] = (u8)((u8)shifted_X[i] ^ (u8)Y[i]);
	}
	u8 shifted_X_xor_Y_addition_C[0x10];
	u8 carry = 0;
	for(i=0;i<16;i++){
		//Type casts. Type casts everywhere.
		shifted_X_xor_Y_addition_C[15-i] = (u8)((u8)shifted_X_xor_Y[15-i] + (u8)carry + (u8)C[15-i]);
		carry = (u8)(((u16)((u8)shifted_X_xor_Y[15-i] + (u8)carry + (u8)C[15-i])) >> (u8)8);
	}
	for(i=0;i<16;i++){
		//Just make it stop! Oh wait, there's no more type casts and masks after this loop.
		if(i < 5) normal_key[i] = (u8)((((u8)shifted_X_xor_Y_addition_C[10+i] << (u8)7)&0x80) + (((u8)shifted_X_xor_Y_addition_C[10+i+1] >> (u8)1)&0x7F))&0xff;
		else if (i == 5) normal_key[i] = (u8)((((u8)shifted_X_xor_Y_addition_C[10+i] << (u8)7)&0x80) + (((u8)shifted_X_xor_Y_addition_C[0] >> (u8)1)&0x7F))&0xff;
		else normal_key[i] = (u8)((((u8)shifted_X_xor_Y_addition_C[-6+i] << (u8)7)&0x80) + (((u8)shifted_X_xor_Y_addition_C[-6+i+1] >> (u8)1)&0x7F))&0xff;
	}
};
