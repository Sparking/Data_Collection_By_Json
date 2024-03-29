﻿#pragma once

#include "json_data_logger_types.h"
#ifdef __cplusplus
extern "C" {
#endif

#define JSON_MAX_QR_COUNT   15   /* 一个JSPON文件最多能记录的QR码的数量 */

typedef struct {
    int order;              /* 序号 */
    json_point_t center;    /* 中心点坐标 */
    json_point_t corners[4];/* 4个角点的坐标点, 0是背向码区, 顺时针顺序存放 */

    /**其他信息**/
    float module_size;      /* 模块大小 */
    int x_size;             /* X宽度 */
    int y_size;             /* Y宽度 */
    int belval;             /* 置信度 */
} json_qr_position_markings;

typedef struct {
    json_point_t relpos;    /* 在QR二维码中相对坐标 */
    json_point_t center;    /* 在图中的中心点坐标 */
} json_qr_alignment_markings;

typedef struct {
    /**位置探测图形信息**/
    int pm_size;                  /* 位置探测图形个数 */
    json_qr_position_markings ppm[3];   /* 位置探测图形信息 */

    /**校正图形信息**/
    int am_size;                  /* 校正图形个数 */
    json_qr_alignment_markings pam[49];  /* 校正图形信息 */

    /**解码信息**/
    /* char *text; */             /* 解码后的数据, 无需记录, 因为有二进制的内容 */
    int text_length;              /* 解码后的数据长度 */
    int codewords_num;            /* 码字总数 */
    int error_codewords;          /* 错误的码字数 */

    /**QR码版本信息和格式信息**/
    unsigned char version;        /* 版本号 1-40 */
    unsigned char ec_level;       /* 纠错级别 0:L, 1:M, 2:Q, 3:H */
    unsigned char mask;           /* 掩膜版本 */

    /**QR码额外信息**/
    bool available;               /* QR是否可用，即是否能被解码 */
    bool inverse;                 /* QR码是否反向 */
    bool mirror;                  /* QR码是否是反相的 */
    unsigned char pm_grayT;       /* 位置探测图形参考灰度值 */

    /**QR码的版本**/
    unsigned char model;          /* QR码版本, 0xFF: 未知版本, 1: QR Model1, 2: QR Model2, 0: MarcoQR */
} json_qr_code_info;

extern void json_qr_info_init(json_qr_code_info *info,
        const unsigned int info_count);

extern void json_qr_info_clear(json_qr_code_info *info,
        const unsigned int info_count);

extern int json_qr_code_info_writer(const char *filename,
        const json_qr_code_info *info, const unsigned int count);

extern int json_qr_code_info_parser(const char *filename,
        json_qr_code_info *info, const unsigned int count);

#ifdef __cplusplus
}

/**解析json对象中的QR码信息, 如果是QR码, 返回0, 否则返回-1**/
extern int json_parse_qr_object(const rapidjson::Value &v, json_qr_code_info *info);

/**将QR码信息设置为一串json个数的字符串,是一个数组**/
extern std::string json_qr_code_info_to_json_string(const json_qr_code_info *info,
        const unsigned int info_count);

#endif

