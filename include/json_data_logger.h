#pragma once

#include "json_qr_info_logger.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	json_codetype_flag_t type;
    // 1d
    // aztec
    // dm
    // dotcode
    // gm
    // hanxin
    // maxi
    // micropdf417
    // ocr_pvi
    // passport
    // pdf417
    // postal
    /* QR Code */
	json_qr_code_info *info_qr;
	unsigned int qr_count;
} json_data_logger_t;

extern void json_data_logger_clear(json_data_logger_t *logger);
extern int json_data_logger_parse_file(const char *filename,
		json_data_logger_t *info);
extern int json_data_logger_writer(const char *std_log_file,
		const char *new_filename, const json_data_logger_t *log);

#ifdef __cplusplus
}
#endif
