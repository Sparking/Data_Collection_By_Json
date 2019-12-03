#include <time.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <json_data_logger.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

json_codetype_flag_t g_json_codetype_flag = {0};

extern "C" void json_data_logger_clear(json_data_logger_t *logger)
{
    if (logger != nullptr) {
        if (logger->info_qr != nullptr) {
            json_qr_info_clear(logger->info_qr, logger->qr_count);
            delete[] logger->info_qr;
        }
        // 释放其他码配置
        memset(logger, 0, sizeof(*logger));
    }
}

/* a是标准 */
std::string json_data_logger_compare(const json_data_logger_t *std_log,
        const json_data_logger_t *b)
{
    std::string diff;
    char diff_log[2048];

    if (std_log == nullptr || b == nullptr) {
        diff = "all\n";
        return diff;
    }

#if 0
    if (a->type.qr != b->type.qr) {
        if (a->type.qr) {
            diff += "Type: Lack of QR\n";
        } else {
            diff += "Type: Find extra QR\n";
        }
    } else if (0) { // 其他码扩展
        diff += "Type: Other Code\n";
    }
#endif

    if (std_log->qr_count != b->qr_count) {
        if (std_log->qr_count > b->qr_count) {
            snprintf(diff_log, sizeof(diff_log), "count: less QR count[%d]\n",
                std_log->qr_count - b->qr_count);
        } else {
            snprintf(diff_log, sizeof(diff_log), "count: more QR count[%d]\n",
                b->qr_count - std_log->qr_count);
        }
        diff += diff_log;
    }

    return diff;
}

extern "C" int json_data_logger_parse_file(const char *filename, json_data_logger_t *info)
{
    unsigned int i, cnt;
    std::fstream json_file;
    rapidjson::Document doc;
    json_qr_code_info qr_info;
    std::vector<json_qr_code_info> vec_qr;

    if (filename == nullptr || info == nullptr)
        return -1;

    memset(info, 0, sizeof(*info));
    json_file.open(filename, std::fstream::in);
    if (!json_file.is_open())
        return -1;

    doc.Parse(std::string((std::istreambuf_iterator<char>(json_file)),
            std::istreambuf_iterator<char>()).c_str());
    json_file.close();
    switch (doc.GetType()) {
    case rapidjson::kArrayType:
        do {
            rapidjson::Document::Array m = doc.GetArray();
            rapidjson::Document::ValueIterator it = m.Begin();

            while (it != m.End()) {
                if (json_parse_qr_object(*it, &qr_info) == 0) {
                    vec_qr.push_back(qr_info);
                } else { // 其他码判断扩展
                }
                ++it;
            }
        } while (0);
        break;
    default:
        return -1;
    }

    /* QR Code */
    cnt = vec_qr.size();
    if (cnt == 0) {
        info->type.qr = false;  /* 配置中没有QR码的信息 */
    } else {
        info->info_qr = new json_qr_code_info[cnt];
        if (info->info_qr == nullptr) {
            json_data_logger_clear(info);
            return -1;
        }

        info->qr_count = cnt;
        info->type.qr = true;
        for (i = 0; i < cnt; ++i)
            memcpy(info->info_qr + i, &vec_qr[i], sizeof(info->info_qr[0]));
    }

    /* 其他码 */
    return 0;
}

static int json_data_logger_save_vector(const char *new_filename,
        std::vector<std::string> &vec_log)
{
    unsigned int i;
    std::fstream fs;
    rapidjson::Document doc;
    rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(s);
    rapidjson::Value jarray(rapidjson::kArrayType);

    if (new_filename == nullptr)
        return -1;

    fs.open(new_filename, std::ios_base::out | std::ios_base::trunc);
    if (!fs.is_open())
        return -1;

    for (i = 0; i < vec_log.size(); ++i) {
        doc.Parse(vec_log[i].c_str());
        if (doc.GetType() == rapidjson::kArrayType) {
            rapidjson::Document::ValueIterator it = doc.GetArray().Begin();
            while (it != doc.GetArray().End()) {
                jarray.PushBack(*it, allocator);
                ++it;
            }
        } else if (doc.GetType() == rapidjson::kObjectType) {
            jarray.PushBack(doc.GetObject(), allocator);
        }
    }
    jarray.Accept(w);
    fs << s.GetString() << std::endl;
    fs.close();

    return 0;
}

/* 返回差异: -1:表示出错 0: 无差异 1: 存在差异 */
extern "C" int json_data_logger_writer(const char *base_file, const char *ba,
        const json_data_logger_t *log)
{
    json_data_logger_t std_log; /* 标准记录文件的解析内容 */
    time_t tmt;
    struct tm cur_tm;
    char new_filename[2048];
    std::fstream diff_fs, new_fs;
    std::string slog;
    std::vector<std::string> vec_log;

    time(&tmt);
#ifdef __linux__
    localtime_r((time_t *)&tmt, &cur_tm);
#else
    localtime_s(&cur_tm, (time_t *)&tmt);
#endif
    snprintf(new_filename, sizeof(new_filename),
        "%s.%04d-%02d-%02d.%02d.%02d.%02d.json",
        base_file, cur_tm.tm_year + 1900, cur_tm.tm_mon, cur_tm.tm_mday,
        cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec);

    if (base_file == nullptr || new_filename == nullptr)
        return -1;

    std::string sfilename = std::string(base_file) + ".std.json";
    if (json_data_logger_parse_file(sfilename.c_str(), &std_log) == -1) {
        return -1;
    }

    sfilename = std::string(base_file) + ".diff.txt";
    diff_fs.open(sfilename.c_str(), std::ios_base::out);
    if (!diff_fs.is_open()) {
        json_data_logger_clear(&std_log);
        std::cerr << "can't log the difference of file \"" << base_file << "\"" << std::endl;
        return -1;
    }
    std::string s = json_data_logger_compare(&std_log, log);
    if (s.size() != 0) {
        diff_fs << "result file: " << new_filename << std::endl;
        diff_fs << s << std::endl;
    }
    diff_fs.close();
    json_data_logger_clear(&std_log);

    slog = json_qr_code_info_to_json_string(log->info_qr, log->qr_count);
    if (slog.size() != 0)
        vec_log.push_back(slog);

    return json_data_logger_save_vector(new_filename, vec_log);
}
