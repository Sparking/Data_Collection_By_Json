﻿#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct  {
    int x;
    int y;
} json_point_t;

typedef struct {
    int order;          // 序号
    json_point_t center;// 中心点坐标
    json_point_t corners[4]; // 4个角点的坐标点, 0是背向码区, 顺时针顺序存放

    // 其他信息
    float module_size;  // 模块大小
    int x_size;         // X宽度
    int y_size;         // Y宽度
    int belval;         // 置信度
} json_qr_position_markings;

typedef struct {
    json_point_t relpos;    // 在QR二维码中相对坐标
    json_point_t center;    // 在图中的中心点坐标
} json_qr_alignment_markings;

typedef struct {
    // 位置探测图形信息
    int pm_size;                  // 位置探测图形个数
    json_qr_position_markings *ppm;   // 位置探测图形信息

    // 校正图形信息
    int am_size;                  // 校正图形个数
    json_qr_alignment_markings *pam;  // 校正图形信息

    // 解码信息
    char *text;                   // 解码后的数据
    int text_length;                // 解码后的数据长度
    int codewords_num;            // 码字总数
    int error_codewords;          // 错误的码字数
    int correct_codewords;        // 纠正的错误码字数
    int mode;                     // 位流的解码模式

    // QR码版本信息和格式信息
    unsigned char version;        // 版本号 0-40
    unsigned char ec_level;       // 纠错级别 0:L, 1:M, 2:Q, 3:H
    unsigned char mask;           // 掩膜版本

    // QR码额外信息
    bool avaiable;                // QR是否可用，即是否能被解码
    bool inverse;                 // QR码是否反向
    bool mirror;                  // QR码是否是反相的
    unsigned char pos_style;      // QR摆放顺序
    unsigned char pm_grayT;       // 位置探测图形参考灰度值
} json_qr_code_info;

extern json_qr_code_info g_json_qr_info[5];
extern size_t g_json_qr_info_count;

static inline void json_qr_info_clear(void)
{
    memset(g_json_qr_info, 0, sizeof(g_json_qr_info));
    g_json_qr_info_count = 0;
}

extern int (*const json_qr_code_info_writer)(const char *filename,
        const json_qr_code_info *info, const size_t count);


#ifdef __cplusplus
}
#endif