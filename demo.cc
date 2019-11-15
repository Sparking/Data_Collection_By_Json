#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct point {
    int x;
    int y;
};

struct qr_position_markings {
    int order;          // 序号
    point center;       // 中心点坐标

// 其他信息
    float module_size;  // 模块大小
    int max_size;       // 最大宽度
    int min_size;       // 最小宽度
};

struct qr_alignment_markings {
    point relpos;       // 在QR二维码中相对坐标
    point center;       // 中心点坐标
};

struct qr_code_info {
    int pm_size;                  // 位置探测图形个数
    qr_position_markings *ppm;    // 位置探测图形信息

    int am_size;                  // 校正图形个数
    qr_alignment_markings *pam;   // 校正图形信息

    bool avaiable;                // QR是否可用，即是否能被解码
    bool inverse;                 // QR码是否反向

    int error_codewords;          // 错误的码字数
    int correct_codewords;        // 纠正的错误码字数
    int data_size;                // 解码后的数据长度
    char *data;                   // 解码后的数据

    int version;                  // 版本号
    int ec_level;                 // 纠错级别
    int mask;                     // 掩膜版本
};

qr_code_info *block_reader(rapidjson::Value &v)
{
    rapidjson::Document::ConstMemberIterator itr, itr1, itr2;
    qr_code_info *info;
    std::vector<qr_position_markings> vqpm;
    std::vector<qr_alignment_markings> vqam;

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

    itr1 = itr->value.FindMember("Position Markings");
    if (itr1 == itr->value.MemberEnd()) {
        std::cerr << "Code Info Should contain Position Markings info at least" << std::endl;
        return nullptr;
    }

    if (itr1->value.GetType() != rapidjson::kArrayType) {
        std::cerr << "Code Info should be an object" << std::endl;
        return nullptr;
    }

    info = new qr_code_info;
    if (info == nullptr)
        return nullptr;

    std::memset(info, 0, sizeof(*info));
    vqpm.clear();
    for (rapidjson::Document::ConstValueIterator vitr = itr1->value.Begin();
            vitr != itr1->value.End(); ++vitr) {
        int flag = 0x3;
        qr_position_markings qpm;

        std::memset(&qpm, 0, sizeof(qpm));
        for (itr2 = vitr->MemberBegin(); itr2 != vitr->MemberEnd(); ++itr2) {
            // position markings的成员center{.x, .y}都必须要配置
            if (std::strcmp(itr2->name.GetString(), "Center X") == 0) {
                qpm.center.x = itr2->value.GetInt();
                flag &= ~0x01L;
            } else if (std::strcmp(itr2->name.GetString(), "Center Y") == 0) {
                qpm.center.y = itr2->value.GetInt();
                flag &= ~0x02L;
            } else if (std::strcmp(itr2->name.GetString(), "Module Size") == 0) {
                qpm.module_size = itr2->value.GetFloat();
            } else if (std::strcmp(itr2->name.GetString(), "Order") == 0) {
                qpm.order = itr2->value.GetInt();
            } else { // 扩展部分
                continue;
            }
        }

        if (!flag) {
            vqpm.push_back(qpm);
        }
    }
    if (vqpm.size() == 0) {
        delete info;
        return nullptr;
    }

    info->pm_size = vqpm.size();
    info->ppm = new qr_position_markings[info->pm_size];
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
            int flag = 0x3;
            qr_alignment_markings qam;

            std::memset(&qam, 0, sizeof(qam));
            for (itr2 = vitr->MemberBegin(); itr2 != vitr->MemberEnd(); ++itr2) {
                // position markings的成员center{.x, .y}都必须要配置
                if (std::strcmp(itr2->name.GetString(), "Center X") == 0) {
                    qam.center.x = itr2->value.GetInt();
                    flag &= ~0x01L;
                } else if (std::strcmp(itr2->name.GetString(), "Center Y") == 0) {
                    qam.center.y = itr2->value.GetInt();
                    flag &= ~0x02L;
                } else if (std::strcmp(itr2->name.GetString(), "Origin X") == 0) {
                    qam.relpos.x = itr2->value.GetInt();
                } else if (std::strcmp(itr2->name.GetString(), "Origin X") == 0) {
                    qam.relpos.y = itr2->value.GetInt();
                } else { // 扩展部分
                    continue;
                }
            }

            if (!flag) {
                vqam.push_back(qam);
            }
        }

        if (vqam.size() != 0) {
            info->pam = new qr_alignment_markings[vqam.size()];
            if (info->pam != nullptr) {
                info->am_size = vqam.size();
                for (size_t i = 0; i < vqam.size(); ++i)
                    std::memcpy(info->pam + i, &vqam[i], sizeof(info->pam[0]));
            }
        }
    } /* End of If */

    return info;
}

int qr_code_info_writer(const char *filename, const std::vector<qr_code_info *> &info)
{
    std::fstream json_file;
    rapidjson::Document doc;
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    qr_code_info *pinfo;

    if (filename == nullptr || info.size() == 0)
        return -1;

    doc.SetArray();
    for (size_t j, i = 0; i < info.size(); ++i) {
        rapidjson::Document code(rapidjson::kObjectType);
        rapidjson::Value vinfo;
        rapidjson::Value qr_apm_info_v, qr_code_info_obj;

        pinfo = info[i];
        if (pinfo == nullptr)
            continue;

        if (pinfo->pm_size <= 0 || pinfo->ppm == nullptr)
            continue;

        vinfo.SetString("QR Code");
        code.AddMember("Code Type", vinfo, doc.GetAllocator());
        vinfo.SetArray();
        vinfo.Clear();
        for (j = 0; j < (size_t)pinfo->pm_size; ++j) {
            qr_code_info_obj.SetObject();
            qr_apm_info_v.SetInt(pinfo->ppm[i].center.x);
            qr_code_info_obj.AddMember("Center X", qr_apm_info_v, doc.GetAllocator());
            qr_apm_info_v.SetInt(pinfo->ppm[i].center.y);
            qr_code_info_obj.AddMember("Center Y", qr_apm_info_v, doc.GetAllocator());
            vinfo.PushBack(qr_code_info_obj, doc.GetAllocator());
        }
        code.AddMember("Code Info", vinfo, doc.GetAllocator());
        doc.PushBack(code, doc.GetAllocator());
    }

    doc.Accept(writer);
    json_file.open(filename, std::fstream::out);
    if (!json_file.is_open()) {
        return -1;
    }

    json_file << buf.GetString();
    json_file.close();

    return 0;
}

int main(int argc, char *argv[])
{
    std::string vstr;
    std::fstream json_file;
    rapidjson::Document doc;
    std::vector<qr_code_info *> info;
    qr_code_info *pinfo;

    if (argc < 2)
        return -1;

    json_file.open(argv[1], std::fstream::in);
    if (!json_file.is_open())
        return -1;

    doc.Parse(std::string((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>()).c_str());
    json_file.close();
    switch (doc.GetType()) {
    case rapidjson::kObjectType:
        {
            rapidjson::Value v = doc.GetObject();
            block_reader(v);
        }
        break;
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
    case rapidjson::kNullType:
    default:
        return -1;
    }

    qr_code_info_writer("test.json", info);
    for (size_t i = 0; i < info.size(); ++i) {
        pinfo = info[i];
        if (pinfo != nullptr) {
            delete[] pinfo->ppm;
            delete[] pinfo->pam;
            delete[] pinfo->data;
            delete[] pinfo;
        }
    }

	return 0;
}
