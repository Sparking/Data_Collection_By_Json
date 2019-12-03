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
#include <rapidjson/ostreamwrapper.h>

extern "C" {
json_qr_code_info g_json_qr_info[JSON_MAX_QR_COUNT];
unsigned int g_json_qr_info_count = 0;
}

extern "C" void json_qr_info_clear(void)
{
    while (g_json_qr_info_count-- > 0) {
        if (g_json_qr_info[g_json_qr_info_count].pam) {
            delete[] g_json_qr_info[g_json_qr_info_count].pam;
        }

        if (g_json_qr_info[g_json_qr_info_count].ppm) {
            delete[] g_json_qr_info[g_json_qr_info_count].ppm;
        }
    }
    memset(g_json_qr_info, 0, sizeof(g_json_qr_info));
    g_json_qr_info_count = 0;
}

extern "C" int json_qr_code_info_writer(const char *filename,
        const json_qr_code_info *info, const unsigned int info_count)
{
    time_t tmt;
    struct tm cur_tm;
    char date[100];
    unsigned int i;
    std::fstream json_file;
    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

    if (filename == nullptr || info == nullptr || info_count == 0)
        return -1;

    time(&tmt);
#ifdef __linux__
    localtime_r((time_t *)&tmt, &cur_tm);
#else
    localtime_s(&cur_tm, (time_t *)&tmt);
#endif
#if 1
    snprintf(date, sizeof(date), "%s.%04d-%02d-%02d.%02d.%02d.%02d.json",
        filename, cur_tm.tm_year + 1900, cur_tm.tm_mon, cur_tm.tm_mday,
        cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec);

    json_file.open(date, std::fstream::out);
    if (!json_file.is_open()) {
        return -1;
    }
#else
    snprintf(date, sizeof(date), "%04d-%02d-%02d %02d:%02d:%02d",
        cur_tm.tm_year + 1900, cur_tm.tm_mon, cur_tm.tm_mday,
        cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec);

    json_file.open(filename, std::ios_base::app | std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    if (!json_file.is_open()) {
        return -1;
    }
    doc.Parse(std::string((std::istreambuf_iterator<char>(json_file)),
            std::istreambuf_iterator<char>()).c_str());
    if (doc.GetType() == rapidjson::kNullType) {
        doc.Parse("[]");
        (void)allocator;
    }
    json_file.close();
    if (doc.GetType() != rapidjson::kArrayType) {
        return -1;
    }
    json_file.open(filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    if (!json_file.is_open()) {
        return -1;
    }
#endif

    writer.StartArray();
    for (i = 0; i < info_count; ++i) {
        if (info[i].pm_size <= 0)
            continue;

        writer.StartObject();
        writer.Key("Code Type");
        writer.String("QR Code");
        writer.Key("Code Info");
        writer.StartObject();
        writer.Key("Model");
        writer.Uint(info[i].model);
        writer.Key("Avaiable");
        writer.Bool(info[i].available);
        writer.Key("Mirror");
        writer.Bool(info[i].mirror);
        writer.Key("Inverse");
        writer.Bool(info[i].inverse);
        writer.Key("Version");
        writer.Uint(info[i].version);
        writer.Key("Error Correction Level");
        writer.Uint(info[i].ec_level);
        writer.Key("Mask");
        writer.Uint(info[i].mask);
        writer.Key("PM Ref Gray");
        writer.Uint(info[i].pm_grayT);
        writer.Key("Decode Info");
        writer.StartObject();
        writer.Key("Text Length");
        writer.Uint(info[i].text_length);
        writer.Key("Codewords Total Size");
        writer.Uint(info[i].codewords_num);
        writer.Key("Error Codewords");
        writer.Uint(info[i].error_codewords);
        writer.EndObject();
        writer.Key("Position Markings");
        writer.StartArray();
        for (int pn = 0; pn < info[i].pm_size; ++pn) {
            const json_qr_position_markings &pm = info[i].ppm[pn];

            writer.StartObject();
            writer.Key("Order");
            writer.Uint(pm.order);
            writer.Key("Center XY");
            writer.StartArray();
            writer.Uint(pm.center.x);
            writer.Uint(pm.center.y);
            writer.EndArray();
            writer.Key("Conners");
            writer.StartArray();
            for (int cn = 0; cn < 4; ++cn) {
                writer.StartArray();
                writer.Uint(pm.corners[cn].x);
                writer.Uint(pm.corners[cn].y);
                writer.EndArray();
            }
            writer.EndArray();
            writer.Key("X Width");
            writer.Uint(pm.x_size);
            writer.Key("Y Width");
            writer.Uint(pm.y_size);
            writer.Key("Module Size");
            writer.Double(pm.module_size);
            writer.EndObject();
        }
        writer.EndArray();
        writer.Key("Alignment Markings");
        writer.StartArray();
        for (int pn = 0; pn < info[i].am_size; ++pn) {
            const json_qr_alignment_markings &am = info[i].pam[pn];

            writer.StartObject();
            writer.Key("Origin XY");
            writer.StartArray();
            writer.Uint(am.relpos.x);
            writer.Uint(am.relpos.y);
            writer.EndArray();
            writer.Key("Center XY");
            writer.StartArray();
            writer.Uint(am.center.x);
            writer.Uint(am.center.y);
            writer.EndArray();
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
        writer.EndObject();
    }
    writer.EndArray();
    json_file << s.GetString() << std::endl;
    json_file.close();

    return 0;
}

#if 1
static json_qr_code_info *block_reader(rapidjson::Value &v)
{
    json_qr_code_info *info;
    std::vector<json_qr_position_markings> vqpm;
    std::vector<json_qr_alignment_markings> vqam;
    rapidjson::Document::ConstMemberIterator itr, itr1, itr2;

    itr = v.FindMember("Code Type");
    if (itr == v.MemberEnd()) {
        std::cerr << "Can't find member [Code Type]" << std::endl;
        return nullptr;
    }

    if (itr->value.GetType() != rapidjson::kStringType
            || std::strcmp(itr->value.GetString(), "QR Code") != 0) {
        std::cerr << "Code Type is not QR Code" << std::endl;
        return nullptr;
    }

    itr = v.FindMember("Code Info");
    if (itr == v.MemberEnd()) {
        std::cerr << "Can't find member [Code Info]" << std::endl;
        return nullptr;
    }

    if (itr->value.GetType() != rapidjson::kObjectType) {
        std::cerr << "Code Info should be an object" << std::endl;
        return nullptr;
    }

    info = new json_qr_code_info;
    if (info == nullptr)
        return nullptr;
    std::memset(info, 0, sizeof(*info));

    itr1 = itr->value.FindMember("Model");
    if (itr1 == itr->value.MemberEnd()
            || (itr1->value.GetType() != rapidjson::kNumberType)) {
        info->model = 0;
    } else {
        info->model = itr1->value.GetUint();
    }

    itr1 = itr->value.FindMember("Avaiable");
    if (itr1 == itr->value.MemberEnd() || (itr1->value.GetType() != rapidjson::kFalseType
            && itr1->value.GetType() != rapidjson::kTrueType)) {
       // 没有设置, 则默认不可解码
        info->available = false;
    } else {
        info->available = itr1->value.GetBool();
    }

    itr1 = itr->value.FindMember("Mirror");
    if (itr1 == itr->value.MemberEnd() || (itr1->value.GetType() != rapidjson::kFalseType
            && itr1->value.GetType() != rapidjson::kTrueType)) {
       // 没有设置, 则默认为非镜像
        info->mirror = false;
    } else {
        info->mirror = itr1->value.GetBool();
    }

    itr1 = itr->value.FindMember("Inverse");
    if (itr1 == itr->value.MemberEnd() || (itr1->value.GetType() != rapidjson::kFalseType
            && itr1->value.GetType() != rapidjson::kTrueType)) {
       // 没有设置, 则默认为非反相
        info->inverse = false;
    } else {
        info->inverse = itr1->value.GetBool();
    }

    itr1 = itr->value.FindMember("Version");
    if (itr1 == itr->value.MemberEnd() || itr1->value.GetType() != rapidjson::kNumberType) {
        info->version = 0; // 0表示QR的版本没有设置, 或设置错误
    } else {
        info->version = itr1->value.GetUint();
        if (info->version > 40)
            info->version = 0;  // 版本不正常, 认为设置错误
    }

    itr1 = itr->value.FindMember("Error Correction Level");
    if (itr1 == itr->value.MemberEnd() || itr1->value.GetType() != rapidjson::kNumberType) {
        info->ec_level = 0xFF; // 0xFF表示QR的纠错级别没有设置, 或设置错误
    } else {
        if (itr1->value.GetUint() > 3) {
            info->ec_level = 0xFF;  // 纠错级别不正常, 认为设置错误
        } else {
            info->ec_level = itr1->value.GetUint();
        }
    }

    itr1 = itr->value.FindMember("Mask");
    if (itr1 == itr->value.MemberEnd() || itr1->value.GetType() != rapidjson::kNumberType) {
        info->mask = 0xFF; // 0xFF表示QR的掩膜版本没有设置, 或设置错误
    } else {
        if (itr1->value.GetUint() > 8) {
            info->mask = 0xFF;  // 掩膜版本不正常, 认为设置错误
        } else {
            info->mask = itr1->value.GetUint();
        }
    }

    itr1 = itr->value.FindMember("PM Ref Gray");
    if (itr1 == itr->value.MemberEnd() || itr1->value.GetType() != rapidjson::kNumberType) {
        info->pm_grayT = 0xFF; // 0xFF
    } else {
        info->pm_grayT = itr1->value.GetUint();
    }

    itr1 = itr->value.FindMember("Decode Info");
    if (itr1 == itr->value.MemberEnd() || itr1->value.GetType() != rapidjson::kObjectType) {
        info->text_length = 0;
        info->codewords_num = 0;
        info->error_codewords = 0;
    } else {
        itr2 = itr1->value.FindMember("Text Length");
        if (itr2 != itr1->value.MemberEnd() && itr2->value.GetType() == rapidjson::kNumberType) {
            info->text_length = itr2->value.GetUint();
        } else {
            info->text_length = 0;
        }

        itr2 = itr1->value.FindMember("Codewords Total Size");
        if (itr2 != itr1->value.MemberEnd() && itr2->value.GetType() == rapidjson::kNumberType) {
            info->codewords_num = itr2->value.GetUint();
        } else {
            info->codewords_num = 0;
        }

        itr2 = itr1->value.FindMember("Error Codewords");
        if (itr2 != itr1->value.MemberEnd() && itr2->value.GetType() == rapidjson::kNumberType) {
            info->error_codewords = itr2->value.GetUint();
        } else {
            info->error_codewords = 0;
        }
    }

    itr1 = itr->value.FindMember("Position Markings");
    if (itr1 == itr->value.MemberEnd() || itr1->value.GetType() != rapidjson::kArrayType) {
        std::cerr << "Code Info Should contain Position Markings info at least" << std::endl;
        delete info;
        return nullptr;
    }

    vqpm.clear();
    for (rapidjson::Document::ConstValueIterator vitr = itr1->value.Begin();
            vitr != itr1->value.End(); ++vitr) {
        int flag = 1;
        json_qr_position_markings qpm;

        std::memset(&qpm, 0, sizeof(qpm));
        for (itr2 = vitr->MemberBegin(); itr2 != vitr->MemberEnd(); ++itr2) {
            const char *name = itr2->name.GetString();
            int count = 0;

            if (std::strcmp(name, "Order") == 0
                    && itr2->value.GetType() == rapidjson::kNumberType) {
                qpm.order = itr2->value.GetUint();
            } else if (std::strcmp(name, "Module Size") == 0
                    && itr2->value.GetType() == rapidjson::kNumberType) {
                qpm.module_size = itr2->value.GetFloat();
            } else if (std::strcmp(name, "Center XY") == 0
                    && itr2->value.GetType() == rapidjson::kArrayType) {
                for (rapidjson::Document::ConstValueIterator tmp_vitr = itr2->value.Begin();
                        tmp_vitr != itr2->value.End(); ++tmp_vitr, ++count) {
                    if (tmp_vitr->GetType() != rapidjson::kNumberType) {
                        flag = 0;
                        break;
                    }

                    if (count == 0) {
                        qpm.center.x = tmp_vitr->GetUint();
                    } else {
                        qpm.center.y = tmp_vitr->GetUint();
                    }
                }

                if (count != 2) {
                    flag = 0;
                    break;
                }
            } else if (std::strcmp(name, "Conners") == 0
                    && itr2->value.GetType() == rapidjson::kArrayType) {
                for (rapidjson::Document::ConstValueIterator tmp_vitr1, tmp_vitr = itr2->value.Begin();
                        tmp_vitr != itr2->value.End(); ++tmp_vitr, ++count) {
                    /* TODO: 解析角点信息 */
                    int ec = 0;

                    if (tmp_vitr->GetType() != rapidjson::kArrayType || count >= 4) {
                        flag = 0;
                        break;
                    }

                    for (tmp_vitr1 = tmp_vitr->GetArray().Begin();
                            tmp_vitr1 != tmp_vitr->GetArray().End(); ++tmp_vitr1, ++ec) {
                        if (ec == 0) {
                            qpm.corners[count].x = tmp_vitr1->GetUint();
                        } else if (ec == 1) {
                            qpm.corners[count].y = tmp_vitr1->GetUint();
                        } else {
                            flag = 0;
                            break;
                        }
                    }
                }

                if (count != 4) {
                    flag = 0;
                    break;
                }
            } else if (std::strcmp(name, "X Width") == 0 && itr2->value.GetType() == rapidjson::kNumberType) {
                qpm.x_size = itr2->value.GetUint();
            } else if (std::strcmp(name, "Y Width") == 0 && itr2->value.GetType() == rapidjson::kNumberType) {
                qpm.y_size = itr2->value.GetUint();
            } else { // 扩展部分
                continue;
            }
        }

        if (flag) {
            vqpm.push_back(qpm);
        }
    }

    if (vqpm.size() == 0) {
        delete info;
        return nullptr;
    }

    info->pm_size = vqpm.size();
    info->ppm = new json_qr_position_markings[info->pm_size];
    if (info->ppm == nullptr) {
        delete info;
        return nullptr;
    }
    for (unsigned int i = 0; i < vqpm.size(); ++i)
        std::memcpy(info->ppm + i, &vqpm[i], sizeof(info->ppm[0]));

    /* 校正图形解析区 */
    itr1 = itr->value.FindMember("Alignment Markings");
    if (itr1 != itr->value.MemberEnd()) {
        if (itr1->value.GetType() != rapidjson::kArrayType) {
            std::cerr << "Code Info should be an object" << std::endl;
            return nullptr;
        }

        vqam.clear();
        for (rapidjson::Document::ConstValueIterator vitr = itr1->value.Begin();
                vitr != itr1->value.End(); ++vitr) {
            int flag = 1;
            json_qr_alignment_markings qam;

            std::memset(&qam, 0, sizeof(qam));
            for (itr2 = vitr->MemberBegin(); itr2 != vitr->MemberEnd(); ++itr2) {
                int count = 0;
                const char *name = itr2->name.GetString();
                json_point_t *p;

                if (std::strcmp(name, "Center XY") == 0) {
                    p = &qam.center;
                } else if (std::strcmp(name, "Origin XY") == 0) {
                    p = &qam.relpos;
                } else { // 扩展部分
                    continue;
                }

                for (rapidjson::Document::ConstValueIterator tmp_vitr = itr2->value.Begin();
                        tmp_vitr != itr2->value.End(); ++tmp_vitr, ++count) {
                    if (tmp_vitr->GetType() != rapidjson::kNumberType) {
                        flag = 0;
                        break;
                    }

                    if (count == 0) {
                        p->x = tmp_vitr->GetUint();
                    } else {
                        p->y = tmp_vitr->GetUint();
                    }
                }

                if (count != 2) {
                    flag = 0;
                    break;
                }
            }

            if (flag) {
                vqam.push_back(qam);
            }
        }

        if (vqam.size() != 0) {
            info->pam = new json_qr_alignment_markings[vqam.size()];
            if (info->pam != nullptr) {
                info->am_size = vqam.size();
                for (unsigned int i = 0; i < vqam.size(); ++i)
                    std::memcpy(info->pam + i, &vqam[i], sizeof(info->pam[0]));
            }
        }
    } /* End of If */

    return info;
}

extern "C" int json_qr_code_info_parser(const char *filename)
{
    std::string vstr;
    unsigned int count;
    std::fstream json_file;
    rapidjson::Document doc;
    json_qr_code_info *pinfo;
    std::vector<json_qr_code_info *> info;

    if (filename == nullptr)
        return -1;

    json_file.open(filename, std::fstream::in);
    if (!json_file.is_open())
        return -1;

    doc.Parse(std::string((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>()).c_str());
    json_file.close();
    switch (doc.GetType()) {
    case rapidjson::kArrayType:
        do {
            rapidjson::Document::Array m = doc.GetArray();
            rapidjson::Document::ValueIterator it = m.Begin();

            while (it != m.End()) {
                pinfo = block_reader(*it);
                if (pinfo != nullptr)
                    info.push_back(pinfo);
                ++it;
            }
        } while (0);
        break;
    default:
        return -1;
    }

    if (info.size() >= JSON_MAX_QR_COUNT) {
        count = JSON_MAX_QR_COUNT;
    } else {
        count = info.size();
        if (count == 0) {
            json_qr_info_clear();
            return -1;
        }
    }

    for (g_json_qr_info_count = 0; g_json_qr_info_count < count;
            ++g_json_qr_info_count) {
        memcpy(g_json_qr_info + g_json_qr_info_count,
                info[g_json_qr_info_count], sizeof(g_json_qr_info[0]));
    }

    for (count = 0; count < info.size(); ++count) {
        pinfo = info[count];
        if (pinfo != nullptr) {
            delete pinfo;
        }
    }

    return 0;
}
#endif
