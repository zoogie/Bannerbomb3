/**
 * File: cmac.c
 * Created on: 21 авг. 2015 г.
 * Description:
 * 
 *
 * Author: Roman Savrulin <romeo.deepmind@gmail.com>
 * Copyright: 2015 Roman Savrulin
 * Copying permission statement:
 * 
 *  This file is part of AES-CMAC.
 *
 *  AES-CMAC is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

/****************************************************************/
/* AES-CMAC with AES-128 bit                                    */
/* CMAC     Algorithm described in SP800-38B                    */
/* Author: Junhyuk Song (junhyuk.song@samsung.com)              */
/*         Jicheol Lee  (jicheol.lee@samsung.com)               */
/****************************************************************/

#include "TI_aes_128.h"
#include "aes-cbc-cmac.h"
#include <string.h>

#ifdef DEBUG_CMAC
#include <stdio.h>
void print_hex(const char *str, const unsigned char *buf, int len) {
	int i;

	for (i = 0; i < len; i++) {
		if ((i % BLOCK_SIZE) == 0 && i != 0)
			printf("%s", str);
		printf("%02x", buf[i]);
		if ((i % 4) == 3)
			printf(" ");
		if ((i % BLOCK_SIZE) == LAST_INDEX)
			printf("\n");
	}
	if ((i % BLOCK_SIZE) != 0)
		printf("\n");
}
void print128(const unsigned char *bytes) {
	int j;
	for (j = 0; j < BLOCK_SIZE; j++) {
		printf("%02x", bytes[j]);
		if ((j % 4) == 3)
			printf(" ");
	}
}

void print96(const unsigned char *bytes) {
	int j;
	for (j = 0; j < 12; j++) {
		printf("%02x", bytes[j]);
		if ((j % 4) == 3)
			printf(" ");
	}
}
#endif

/* For CMAC Calculation */
static unsigned const char const_Rb[BLOCK_SIZE] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87 };
static unsigned const char const_Zero[BLOCK_SIZE] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int AES_CMAC_CHECK(const unsigned char *key, const unsigned char *input, int length,
		const unsigned char *mac){
	unsigned char T[BLOCK_SIZE];
	AES_CMAC(key, input, length, T);
	/*print128(T);
	printf("\n");
	print128(mac);*/
	return memcmp(mac, T, BLOCK_SIZE);
}

static void AES_128_ENC(unsigned const char *key, unsigned const char* msg, unsigned char *cipher){
	unsigned char key_copy[BLOCK_SIZE];
	memcpy(cipher, msg, BLOCK_SIZE);
	memcpy(key_copy, key, BLOCK_SIZE);
	aes_enc_dec(cipher, key_copy, 0);
}

void AES_128_DEC(unsigned const char *key, unsigned const char* msg, unsigned char *cipher){
	unsigned char key_copy[BLOCK_SIZE];
	memcpy(cipher, msg, BLOCK_SIZE);
	memcpy(key_copy, key, BLOCK_SIZE);
	aes_enc_dec(cipher, key_copy, 1);
}

void xor_128(const unsigned char *a, const unsigned char *b, unsigned char *out) {
	int i;
	for (i = 0; i < BLOCK_SIZE; i++) {
		out[i] = a[i] ^ b[i];
	}
}

static void padding_AES(const unsigned char *lastb, unsigned char *pad, int length) {
	int j;
	length = length % BLOCK_SIZE;

	if(length == 0){
		memcpy(pad, lastb, BLOCK_SIZE);
		return;
	}

	/* original last block */
	for (j = 0; j < BLOCK_SIZE; j++) {
		if (j < length) {
			pad[j] = lastb[j];
		} else {
			pad[j] = 0x00;
		}
	}
}

int AES_CBC_ENC(const unsigned char *IV, const unsigned char *key, const unsigned char *input, int inputLength, unsigned char *output, int outputLength){
	unsigned char X[BLOCK_SIZE], Y[BLOCK_SIZE], M_last[BLOCK_SIZE];

	if (inputLength <= 0)
		return 0; //nothing to encode

	int n = (inputLength + LAST_INDEX) / BLOCK_SIZE; //TODO: last

	memcpy(X, IV, BLOCK_SIZE);
	padding_AES(&input[BLOCK_SIZE * (n - 1)], M_last, inputLength);

	for (int i = 0; (i < n) && outputLength > 0; i++) {
		unsigned const char * text = &input[BLOCK_SIZE * i];
		if(i == n - 1){
			text = M_last;
		}
		int outLen = (BLOCK_SIZE < outputLength)?BLOCK_SIZE:outputLength;
		xor_128(X, text, Y);
		AES_128_ENC(key, Y, X);
		memcpy(output, X, outLen);
		outputLength -= outLen;
		output += outLen;
	}

	return n * BLOCK_SIZE;
}

int AES_CBC_DEC(const unsigned char *IV, const unsigned char *key, const unsigned char *input, int inputLength, unsigned char *output, int outputLength){
	unsigned char X[BLOCK_SIZE], text[BLOCK_SIZE], Z[BLOCK_SIZE];

	if (inputLength <= 0)
		return 0; //nothing to encode

	inputLength = ( inputLength / BLOCK_SIZE ) * BLOCK_SIZE;

	int n = (inputLength + LAST_INDEX) / BLOCK_SIZE;

	memcpy(Z, IV, BLOCK_SIZE);
	for (int i = 0; (i < n) && outputLength > 0; i++) {
		unsigned const char * cipher = &input[BLOCK_SIZE * i];
		AES_128_DEC(key, cipher, X);
		xor_128(Z, X, text);
		memcpy(Z, cipher, BLOCK_SIZE);
		memcpy(output, text, BLOCK_SIZE);
		outputLength -= BLOCK_SIZE;
		output += BLOCK_SIZE;
	}

	return n * BLOCK_SIZE;
}

/* AES-CMAC Generation Function */

static void leftshift_onebit(const unsigned char *input, unsigned char *output) {
	int i;
	unsigned char overflow = 0;

	for (i = LAST_INDEX; i >= 0; i--) {
		output[i] = input[i] << 1;
		output[i] |= overflow;
		overflow = (input[i] & 0x80) ? 1 : 0;
	}
	return;
}

static void generate_subkey(const unsigned char *key, unsigned char *K1, unsigned
char *K2) {
	unsigned char L[BLOCK_SIZE];
	unsigned char tmp[BLOCK_SIZE];

	AES_128_ENC(key, const_Zero, L);

	if ((L[0] & 0x80) == 0) { /* If MSB(L) = 0, then K1 = L << 1 */
		leftshift_onebit(L, K1);
	} else { /* Else K1 = ( L << 1 ) (+) Rb */

		leftshift_onebit(L, tmp);
		xor_128(tmp, const_Rb, K1);
	}

	if ((K1[0] & 0x80) == 0) {
		leftshift_onebit(K1, K2);
	} else {
		leftshift_onebit(K1, tmp);
		xor_128(tmp, const_Rb, K2);
	}
	return;
}

static void padding(const unsigned char *lastb, unsigned char *pad, int length) {
	int j;

	/* original last block */
	for (j = 0; j < BLOCK_SIZE; j++) {
		if (j < length) {
			pad[j] = lastb[j];
		} else if (j == length) {
			pad[j] = 0x80;
		} else {
			pad[j] = 0x00;
		}
	}
}

void AES_CMAC(const unsigned char *key, const unsigned char *input, int length,
		unsigned char *mac) {
	unsigned char X[BLOCK_SIZE], Y[BLOCK_SIZE], M_last[BLOCK_SIZE], padded[BLOCK_SIZE];
	unsigned char K1[BLOCK_SIZE], K2[BLOCK_SIZE];
	int n, i, flag;
	generate_subkey(key, K1, K2);

	n = (length + LAST_INDEX) / BLOCK_SIZE; /* n is number of rounds */

	if (n == 0) {
		n = 1;
		flag = 0;
	} else {
		if ((length % BLOCK_SIZE) == 0) { /* last block is a complete block */
			flag = 1;
		} else { /* last block is not complete block */
			flag = 0;
		}
	}

	if (flag) { /* last block is complete block */
		xor_128(&input[BLOCK_SIZE * (n - 1)], K1, M_last);
	} else {
		padding(&input[BLOCK_SIZE * (n - 1)], padded, length % BLOCK_SIZE);
		xor_128(padded, K2, M_last);
	}

	memset(X, 0, BLOCK_SIZE);
	for (i = 0; i < n - 1; i++) {
		xor_128(X, &input[BLOCK_SIZE * i], Y); /* Y := Mi (+) X  */
		AES_128_ENC(key, Y, X); /* X := AES-128(KEY, Y); */
	}

	xor_128(X, M_last, Y);
	AES_128_ENC(key, Y, X);

	memcpy(mac, X, BLOCK_SIZE);
}