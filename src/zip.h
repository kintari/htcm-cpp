#pragma once

#define ZIP_FILE_MAGIC  0x04034b50
#define ZIP_EOCD_MAGIC  0x06054b50

#define ZIP_COMPRESSION_METHOD_DEFLATE 8

#pragma pack(push,1)

typedef struct zip_eocd_t {
	uint32_t magic;
	uint16_t unused[3];
	uint16_t num_cdir_recs;
	uint32_t cdir_size, cdir_start;
	uint16_t comment_length;
} zip_eocd_t;

typedef struct zip_cd_file_t {
	uint32_t magic; // 0x02014b50
	uint16_t version_made_by, version_required_to_extract;
	uint16_t flags, compression_method, file_last_mtime, file_last_mdate;
	uint32_t crc;
	uint32_t compressed_size, uncompressed_size;
	uint16_t filename_len, extra_field_len, file_comment_len;
	uint8_t unused[8];
	uint32_t local_file_header;
} zip_cd_file_t;

typedef struct zip_lfh_t {
	uint32_t magic; // 0x04034b50
	uint16_t version_required_to_extract;
	uint16_t flags, compression_method, file_last_mtime, file_last_mdate;
	uint32_t crc, compressed_size, uncompressed_size;
	uint16_t filename_len, extra_field_len;
} zip_lfh_t;

#pragma pack(pop)