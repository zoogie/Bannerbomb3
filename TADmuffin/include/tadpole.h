
#define SIZE_FOOTER 0x4E0
#define SIZE_CTCERTBIN 0x19E

typedef u8 sha256_hash[0x20];

typedef struct ecc_point_t
{
	uint8_t r[0x1e];
	uint8_t s[0x1e];
} __attribute__((packed)) ecc_point_t;

typedef struct ecc_cert_t
{
	struct {
		uint32_t type;
		ecc_point_t val;
		uint8_t padding[0x40];
	} sig;
	char issuer[0x40];
	uint32_t key_type;
	char key_id[0x40];
	uint32_t unk;
	ecc_point_t pubkey;
	uint8_t padding2[0x3c];
} __attribute__((packed)) ecc_cert_t;

typedef struct footer_t
{
	sha256_hash banner_hash;
	sha256_hash hdr_hash;
	sha256_hash tmd_hash;
	sha256_hash content_hash[8];
	sha256_hash savedata_hash;
	sha256_hash bannersav_hash;
	ecc_point_t sig;
	ecc_cert_t ap;
	ecc_cert_t ct;
} __attribute__((packed)) footer_t;

typedef struct header_t
{
	u32 magic;
	u16 group_id;
	u16 version;
	u8 sha256_ivs[0x20];
	u8 aes_zeroblock[0x10];
	u32 tidlow;
	u32 tidhigh;
	u64 installed_size;
	u32 content[11];
	u8 padding[0x7C]; //not totally padding, but no idea what some values in the middle are
} __attribute__((packed)) header_t;

typedef struct banner_t
{
	u16 version;
	u16 crc16s[4];
	u8 pad[0x16];
	u8 icon[0x220]; //bitmap 0x200 + palette 0x20
	u8 bannerstrings[16][0x100];
	u8 dontcare[0x4000-0x1240]; //wide open space for payloads etc., although data past banner + 0x23C0 usually isn't copied to memory.
} __attribute__((packed)) banner_t;

void getSection(u8 *dsiware_pointer, u32 section_size, u8 *key, u8 *output);
void placeSection(u8 *dsiware_pointer, u8 *section, u32 section_size, u8 *key, u8 *key_cmac);