#pragma once

#include "json_qr_info_logger.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    json_codetype_flag_t type;

    /* 1d */
    void *info_1d;
    unsigned int n1d;

    /* aztec */
    void *info_aztec;
    unsigned int naztec;

    /* dm */
    void *info_dm;
    unsigned int ndm;

    /* dotcode */
    void *info_dotcode;
    unsigned int ndotcode;

    /* gm */
    void *info_gm;
    unsigned int ngm;

    /* hanxin */
    void *info_hanxin;
    unsigned int nhanxin;

    /* maxi */
    void *info_maxi;
    unsigned int nmaxi;

    /* micropdf417 */
    void *info_mpdf417;
    unsigned int nmpdf417;

    /* ocr_pvi */
    void *info_ocr_pvi;
    unsigned int nocr_pvi;

    /* passport */
    void *info_passport;
    unsigned int npassport;

    /* pdf417 */
    void *info_pdf417;
    unsigned int npdf417;

    /* postal */
    void *info_postal;
    unsigned int npostal;

    /* QR Code */
    json_qr_code_info *info_qr;
    unsigned int nqr;
} json_data_logger_t;

#ifdef ENABLE_JSON_LOG
extern json_data_logger_t g_json_logger;
extern unsigned int g_json_logger_code_max_cnt;
extern void g_json_logger_init(const unsigned int max_cnt);
extern void g_json_logger_clear(void);
#endif

/**清理内存**/
extern void json_data_logger_clear(json_data_logger_t *logger);

/**解析出json文件中的信息到结构体中,成功返回0,失败返回-1**/
extern int json_data_logger_parse_file(const char *filename,
        json_data_logger_t *info);

/**未完善**/
extern int json_data_logger_writer(const char *std_log_file,
        const char *new_filename, const json_data_logger_t *log);

#ifdef __cplusplus
}
#endif
