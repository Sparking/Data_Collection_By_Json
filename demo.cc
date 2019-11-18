#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <json_data_logger.h>

extern int cpp_qr_code_info_writer(const char *filename,
        const json_qr_code_info *info, const size_t count);

extern "C" {
json_qr_code_info g_json_qr_info[5];
size_t g_json_qr_info_count;
int (*const json_qr_code_info_writer)(const char *filename,
        const json_qr_code_info *info, const size_t count)
    = cpp_qr_code_info_writer;
}

json_qr_code_info *block_reader(rapidjson::Value &v)
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

    itr1 = itr->value.FindMember("Avaiable");
    if (itr1 == itr->value.MemberEnd() || (itr1->value.GetType() != rapidjson::kFalseType
            && itr1->value.GetType() != rapidjson::kTrueType)) {
       // 没有设置, 则默认不可解码
        info->avaiable = false;
    } else {
        info->avaiable = itr1->value.GetBool();
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

    itr1 = itr->value.FindMember("Mode");
    if (itr1 == itr->value.MemberEnd() || itr1->value.GetType() != rapidjson::kNumberType) {
        info->mode = 0xFF; // 0xFF表示QR的掩膜版本没有设置, 或设置错误
    } else {
        info->mode = itr1->value.GetUint();
    }

    itr1 = itr->value.FindMember("Decode Result");
    if (itr1 == itr->value.MemberEnd() || itr1->value.GetType() != rapidjson::kObjectType) {
        info->text = nullptr;
        info->text_length = 0;
        info->codewords_num = 0;
        info->error_codewords = 0;
        info->correct_codewords = 0;
    } else {
        itr2 = itr1->value.FindMember("Text");
        if (itr2 != itr1->value.MemberEnd() && itr2->value.GetType() == rapidjson::kStringType) {
            info->text = strdup(itr2->value.GetString());
        } else {
            info->text = nullptr;
        }

        itr2 = itr1->value.FindMember("Text Length");
        if (itr2 != itr1->value.MemberEnd() && itr2->value.GetType() == rapidjson::kNumberType) {
            info->text_length = itr2->value.GetUint();
        } else if (info->text != nullptr) {
            info->text_length = strlen(info->text);
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

        itr2 = itr1->value.FindMember("Corrected Codewords");
        if (itr2 != itr1->value.MemberEnd() && itr2->value.GetType() == rapidjson::kNumberType) {
            info->correct_codewords = itr2->value.GetUint();
        } else {
            info->correct_codewords = info->error_codewords;
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

            if (std::strcmp(name, "Order") == 0 && itr2->value.GetType() == rapidjson::kNumberType) {
                qpm.order = itr2->value.GetUint();
            } else if (std::strcmp(name, "Module Size") == 0 && itr2->value.GetType() == rapidjson::kNumberType) {
                qpm.module_size = itr2->value.GetFloat();
            } else if (std::strcmp(name, "Center XY") == 0 && itr2->value.GetType() == rapidjson::kArrayType) {
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
            } else if (std::strcmp(name, "Conners") == 0 && itr2->value.GetType() == rapidjson::kArrayType) {
                for (rapidjson::Document::ConstValueIterator tmp_vitr1, tmp_vitr = itr2->value.Begin();
                        tmp_vitr != itr2->value.End(); ++tmp_vitr, ++count) {
                    /* TODO: 解析角点信息 */
                    int ec = 0;

                    if (tmp_vitr->GetType() != rapidjson::kArrayType || count >= 4) {
                        flag = 0;
                        break;
                    }

                    for (tmp_vitr1 = tmp_vitr->GetArray().Begin(); tmp_vitr1 != tmp_vitr->GetArray().End(); ++tmp_vitr1, ++ec) {
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
    for (size_t i = 0; i < vqpm.size(); ++i)
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
                for (size_t i = 0; i < vqam.size(); ++i)
                    std::memcpy(info->pam + i, &vqam[i], sizeof(info->pam[0]));
            }
        }
    } /* End of If */

    return info;
}

int cpp_qr_code_info_writer(const char *filename, const json_qr_code_info *info, const size_t info_count)
{
    size_t i;
    std::fstream json_file;
    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

    if (filename == nullptr || info == nullptr || info_count == 0)
        return -1;

    json_file.open(filename, std::fstream::out);
    if (!json_file.is_open()) {
        return -1;
    }

    writer.StartArray();
    for (i = 0; i < info_count; ++i) {
        if (info[i].pm_size <= 0)
            continue;

        writer.StartObject();
        writer.Key("Code Type");
        writer.String("QR Code");
        writer.Key("Code Info");
        writer.StartObject();
        writer.Key("Avaiable");
        writer.Bool(info[i].avaiable);
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
        writer.Key("Mode");
        writer.Uint(info[i].mode);
        writer.Key("Decode Result");
        writer.StartObject();
        writer.Key("Text");
        if (info[i].text == nullptr) {
            writer.Null();
        } else {
            writer.String(info[i].text);
        }
        writer.Key("Text Length");
        writer.Uint(info[i].text_length);
        writer.Key("Codewords Total Size");
        writer.Uint(info[i].codewords_num);
        writer.Key("Error Codewords");
        writer.Uint(info[i].error_codewords);
        writer.Key("Corrected Codewords");
        writer.Uint(info[i].correct_codewords);
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

    json_file << s.GetString();
    json_file.close();

    std::cout << s.GetString() << std::endl;

    return 0;
}

int main(int argc, char *argv[])
{
    std::string vstr;
    std::fstream json_file;
    rapidjson::Document doc;
    std::vector<json_qr_code_info *> info;
    json_qr_code_info *pinfo;

    if (argc < 2)
        return -1;

    json_file.open(argv[1], std::fstream::in);
    if (!json_file.is_open())
        return -1;

    doc.Parse(std::string((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>()).c_str());
    json_file.close();
    switch (doc.GetType()) {
    case rapidjson::kArrayType:
        {
            rapidjson::Document::Array m = doc.GetArray();
            rapidjson::Document::ValueIterator it = m.Begin();

            while (it != m.End()) {
                pinfo = block_reader(*it);
                if (pinfo != nullptr)
                    info.push_back(pinfo);
                ++it;
            }
        }
        break;
    default:
        return -1;
    }

    json_qr_info_clear();
    for (g_json_qr_info_count = 0; g_json_qr_info_count < info.size(); ++g_json_qr_info_count) {
        std::memcpy(g_json_qr_info + g_json_qr_info_count, info[g_json_qr_info_count], sizeof(g_json_qr_info[0]));
    }
    json_qr_code_info_writer("test.json", g_json_qr_info, g_json_qr_info_count);
    for (size_t i = 0; i < info.size(); ++i) {
        pinfo = info[i];
        if (pinfo != nullptr) {
            delete[] pinfo->ppm;
            delete[] pinfo->pam;
            free(pinfo->text);
            delete[] pinfo;
        }
    }

	return 0;
}
