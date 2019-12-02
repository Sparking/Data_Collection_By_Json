#include <json_data_logger.h>

int main(int argc, char *argv[])
{
    json_qr_info_clear();

    if (json_qr_code_info_parser(argv[1]) != 0)
    	return -1;

    json_qr_code_info_writer("test.json",
    		g_json_qr_info, g_json_qr_info_count);
    json_qr_info_clear();

    return 0;
}
