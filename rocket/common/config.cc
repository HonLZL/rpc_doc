// /usr/include/tinyxml
#include <tinyxml/tinyxml.h>
#include <string>
#include "config.h"

// 使用了 # 符号，将 x 参数转换为字符串字面量
// a##b, 使用了 ## 操作符将传入的参数 a 和 b 连接在一起
#define READ_XML_NODE(name, parent)                                            \
    TiXmlElement* name##_node = parent->FirstChildElement(#name);        \
    if (!name##_node) {                                                        \
        printf("Start rocket server error, failed to read node [%s] error info [%s] \n ", #name, xml_document->ErrorDesc()); \
        exit(0);                                                               \
    }

#define READ_STR_FROM_XML_NODE(name, parent)                                        \
    TiXmlElement* name##_node = parent->FirstChildElement(#name);             \
    if (!name##_node || !name##_node->GetText()) {                                  \
        printf("Start rocket server error, failed to read config file %s\n", #name); \
        exit(0);                                                                    \
    } \
    std::string name##_str = std::string(name##_node->GetText()); \

namespace rocket {

static Config* g_config = nullptr;

Config* Config::GetGlobalConfig() {
    return g_config;
}

void Config::SetGlobalConfig(const char* xmlfile) {
    if (g_config == nullptr) {
        g_config = new Config(xmlfile);
    }
}

Config::Config(const char* xmlfile) {
    TiXmlDocument* xml_document = new TiXmlDocument();

    bool rt = xml_document->LoadFile(xmlfile);
    if (!rt) {
        printf("Start rocket server error, rt = %d, failed to read config file %s\n", rt, xmlfile);
        xml_document->ErrorDesc();
        exit(0);
    }

    READ_XML_NODE(root, xml_document);

    READ_XML_NODE(log, root_node);

    READ_STR_FROM_XML_NODE(log_level, log_node);

    m_log_level = log_level_str;

}
}  // namespace rocket
