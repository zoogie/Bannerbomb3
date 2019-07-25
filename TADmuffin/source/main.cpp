#include <cstring>
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "crypto.h"
#include "tadpole.h"
#include "types.h"
#include "payload.h"

u32 Tidlow=0xF00D43D5;

void error(const char *errormsg, const char *filename, bool fatal) {
	printf("%s:%s %s\nHit Enter to close\n", fatal ? "ERROR":"WARNING", errormsg, filename); 
	getchar();
	if(fatal) exit(1); 
}

u8 *readAllBytes(const char *filename, u32 &filelen) {
	FILE *fileptr = fopen(filename, "rb");
	if (fileptr == NULL) {
		fileptr = fopen("movable.sed","rb");
		if (fileptr == NULL) error("Failed to open ", filename, true);
	}
	
	fseek(fileptr, 0, SEEK_END);
	filelen = ftell(fileptr);
	rewind(fileptr);
	
	if(filelen > 0x4000) filelen=0x4000; //keep dsiware buffer reasonable

	u8 *buffer = (u8*)malloc(filelen);

	fread(buffer, filelen, 1, fileptr);
	fclose(fileptr);

	return buffer;
}

void writeAllBytes(const char *filename, u8 *filedata, u32 filelen) {
	FILE *fileptr = fopen(filename, "wb");
	fwrite(filedata, 1, filelen, fileptr);
	fclose(fileptr);
}


u16 crc16(u8 *data, u32 N) //https://modbus.control.com/thread/1381836105#1381859471
{
    u16    reg, bit, yy, flag;

    yy = 0;
    reg = 0xffff;

    while( N-- > 0 ) 
    {
	reg = reg ^ data[yy];
	yy = yy + 1;

	for ( bit = 0; bit <= 7; bit++ )
	{
	    flag = reg & 0x0001;
	    reg  = reg >> 1;
	    if ( flag == 1 )
	        reg = reg ^ 0xa001;
	}
    }
    return ( reg );
}

void fixcrc16(u16 *checksum, u8 *message, u32 len){
	u16 original=*checksum;
	u16 calculated=crc16(message, len);
	printf("orig:%04X calc:%04X  ", original, calculated);
	if(original != calculated){
		*checksum=calculated;
		printf("fixed\n");
		return;
	}
	printf("good\n");
}

void rebuildTad(char *filename, const char *dname) {
	u8 *dsiware, *movable, *altbanner;
	u32 header_size=0xF0, footer_size=0x4E0, movable_size, banner_size=0x4000, altbanner_size;
	//u8 banner_hash[0x20]={0}, header_hash[0x20] = {0};
	//u8 content_hash[11][0x20]={0};
	u32 banner_off=0, header_off=0x4020, footer_off=0x4130;  //0x4000+0x20+0xF0+0x20+0x4E0+0x20
	u32 checked_size=(banner_size+0x20) + (header_size+0x20) + (footer_size+0x20);
	//u32 content_size[11]={0};
	//const char *content_namelist[]={"tmd","srl.nds","2.bin","3.bin","4.bin","5.bin","6.bin","7.bin","8.bin","public.sav","banner.sav"};
	//u8 *contents[11];
	u8 normalKey[0x10] = {0}, normalKey_CMAC[0x10] = {0};
	char outname[512]={0};
	//memset(content_hash, 0, 11*0x20);
	
	printf("Reading movable.sed\n");
	movable = readAllBytes(filename, movable_size);
	if (movable_size != 320 && movable_size != 288) {
		error("Provided movable.sed is not 320 or 288 bytes of size","", true);
	}
	
	
	banner_t Banner;
	header_t Header;
	footer_t Footer;
	
	memset(&Banner.version, 0x77, sizeof(banner_t));
	memset(&Header.magic, 0, sizeof(header_t));
	memset(&Footer.banner_hash, 0, sizeof(footer_t));
	
	u8 *banner=(u8*)calloc(banner_size,1);
	u8 *header=(u8*)calloc(header_size,1);
	u8 *footer=(u8*)calloc(footer_size,1);
	
	
	Banner.version=0x0103;
	Header.magic=0x54444633; //"3FDT"
	Header.version=0x0400;
	memset(&Header.sha256_ivs[0], 0x42, 0x20);
	memset(&Header.aes_zeroblock[0], 0x99, 0x10);
	Header.tidlow=Tidlow;         //0x484E4441
	
	for(int i=0; i<8; i++){
		memcpy(&Banner.bannerstrings[i], payload, payload_size);
	}
	
	memcpy(banner, &Banner.version, sizeof(banner_t));
	memcpy(header, &Header.magic, sizeof(header_t));
	memcpy(footer, &Footer.banner_hash, sizeof(footer_t));
	
	FILE *f = fopen("altbanner.bin", "rb");
	if(f){
		fclose(f);
		altbanner=readAllBytes("altbanner.bin", altbanner_size);
		if(altbanner_size != 0x4000) error("altbanner is not 0x4000 bytes","altbanner.bin",true); 
		banner=altbanner;
	}
	
	printf("Fixing banner crc16s\n");
	fixcrc16((u16*)(banner+0x2), banner+0x20, 0x820);
	fixcrc16((u16*)(banner+0x4), banner+0x20, 0x920);
	fixcrc16((u16*)(banner+0x6), banner+0x20, 0xA20);
	fixcrc16((u16*)(banner+0x8), banner+0x1240, 0x1180);
	
	printf("Scrambling keys\n");
	keyScrambler((movable + 0x110), false, normalKey);
	keyScrambler((movable + 0x110), true, normalKey_CMAC);
	
	printf("Copying all sections to output buffer\n");
	
	dsiware=(u8*)malloc(0x4000+0xF0+0x4E0+(3*0x20));
	printf("Writing banner\n"); placeSection((dsiware + banner_off), banner, banner_size, normalKey, normalKey_CMAC);
	printf("Writing header\n"); placeSection((dsiware + header_off), header, header_size, normalKey, normalKey_CMAC);
	printf("Writing footer\n"); placeSection((dsiware + footer_off), footer, footer_size, normalKey, normalKey_CMAC);
	
	
	snprintf(outname, 256, "%s/%08X.bin", dname, Header.tidlow);

	printf("Writing to:    %s/%08X.bin\n", dname, Header.tidlow);
	writeAllBytes(outname, dsiware, checked_size); 
	printf("Done!\n");
	
	printf("Cleaning up\n");
	free(banner);
	free(header);
	free(footer);
	free(dsiware);
	free(movable);
	printf("Done!\n");
}

void usage(){
	printf("Usage (choose one):\n");
	printf("1. 'TADmuffin movable.sed' at command prompt\n");
	printf("2.  Drag and Drop movable.sed on TADmuffin\n");
	printf("3.  Run TADmuffin with movable.sed beside it\n");
}

int main(int argc, char* argv[]) {
	//char dname[512]={0};
	//u32 tidlow=0x484E4441; //DLP   (or 0x42383841 INT)
	const char *dname1="Usa_Europe_Japan_Korea";
	//const char *dname2="China_Taiwan";
	
	if(argc>3){
		usage();
		return 1;
	}
	
	mkdir(dname1);
	//mkdir(dname2);
	
	printf("|TADmuffin by zoogie|\n");
	printf("|________v1.0_______|\n");
	
	rebuildTad(argv[1], dname1);  
	printf("\nJob completed!\nPress Enter to close\n");
	getchar();
	
	return 0;
}