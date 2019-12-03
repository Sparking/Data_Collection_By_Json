#pragma once

#include <stdbool.h>

#ifdef __cplusplus
#include <rapidjson/document.h>
extern "C" {
#endif

typedef struct  {
    int x;
    int y;
} json_point_t;

typedef struct {
	bool _1dcode;
	bool aztec;
	bool dm;
	bool dotcode;
	bool gm;
	bool hanxin;
	bool maxi;
	bool micropdf417;
	bool ocr_pvi;
	bool passport;
	bool pdf417;
	bool postal;
	bool qr;
} json_codetype_flag_t;

#ifdef __cplusplus
}
#endif
