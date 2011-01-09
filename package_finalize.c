/*
 * package_finalize by geohot part of geohot's awesome tools for the PS3
 * released under GPLv3, see http://gplv3.fsf.org/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ps3_common.h"
#include "oddkeys.h"
#include <openssl/sha.h>
#include <openssl/aes.h>

typedef struct {
	u32             magic;
	u32             debugFlag;
	u32             infoOffset;
	u32             unknown1;
	u32             headSize;
	u32             itemCount;
	u64             packageSize;
	u64             dataOffset;
	u64             dataSize;
}               pkg_header;

int 
main(int argc, char *argv[])
{
	u8             *data;
	u8              ecount_buf[0x10];
	u8              sha_key[0x40];
	u8              sha_crap[0x40];
	FILE           *f;
	FILE           *g;
	pkg_header     *header;
	int             i, nlen, data_size, data_offset, dptr;
	int             num = 0;

	if (argc < 2) {
		printf("usage: %s my.pkg\n", argv[0]);
		return -1;
	}
	f = fopen(argv[1], "rb");
	fseek(f, 0, SEEK_END);
	nlen = ftell(f);
	fseek(f, 0, SEEK_SET);

	data = (u8 *) malloc(nlen);

	fread(data, 1, nlen, f);
	fclose(f);

	header = (pkg_header *) data;
	data_offset = get_u64(&(header->dataOffset));
	data_size = get_u64(&(header->dataSize));

	/* decrypt the debug stuff */
	memset(sha_crap, 0, 0x40);
	memcpy(sha_crap, &data[0x60], 8);
	memcpy(sha_crap + 0x8, &data[0x60], 8);
	memcpy(sha_crap + 0x10, &data[0x68], 8);
	memcpy(sha_crap + 0x18, &data[0x68], 8);


	for (dptr = data_offset; dptr < (data_offset + data_size); dptr += 0x10) {
		u8              hash[0x14];

		SHA1(sha_crap, 0x40, hash);
		for (i = 0; i < 0x10; i++)
			data[dptr + i] ^= hash[i];
		set_u64(sha_crap + 0x38, get_u64(sha_crap + 0x38) + 1);
	}

	/* reencrypt as retail */
	u8              pkg_key[0x10];
	memcpy(pkg_key, &data[0x70], 0x10);

	AES_KEY         aes_key;
	AES_set_encrypt_key(retail_pkg_aes_key, 128, &aes_key);

	memset(ecount_buf, 0, 0x10);
	AES_ctr128_encrypt(&data[data_offset],
			   &data[data_offset],
			   data_size,
			   &aes_key,
			   pkg_key,
			   ecount_buf,
			   &num);

	/* write file back */
	fopen(argv[1], "wb");
	data[4] = 0x80;		/* set finalize flag */
	memset(&data[(data_offset + data_size)], 0, 0x60);

	/* add sha1-hash */
	SHA1(data, nlen - 0x20, &data[nlen - 0x20]);
	fwrite(data, 1, nlen, g);
	fclose(g);
}
