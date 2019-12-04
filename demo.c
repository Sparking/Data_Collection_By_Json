#include <stdio.h>
#include <json_data_logger.h>

int main(int argc, char *argv[])
{
    json_qr_code_info g_json_qr_info[JSON_MAX_QR_COUNT];
    unsigned int g_json_qr_info_count = 0;
    json_data_logger_t a;

    if (argc < 2)
        return -1;

    g_json_qr_info_count = json_qr_code_info_parser(argv[1], g_json_qr_info, JSON_MAX_QR_COUNT);
    json_qr_code_info_writer("test.json", g_json_qr_info, g_json_qr_info_count);
    json_qr_info_clear(g_json_qr_info, g_json_qr_info_count);

    if (json_data_logger_parse_file(argv[1], &a) == -1) {
        json_data_logger_clear(&a);
        fprintf(stderr, "error while parse file %s\n", argv[1]);
        return -1;
    }
    json_data_logger_writer("test1", argv[1], &a);
    json_data_logger_clear(&a);

    return 0;
}
