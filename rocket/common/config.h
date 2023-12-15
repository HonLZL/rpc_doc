#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

namespace rocket {
class Config {
   public:
    Config(const char* xmlfile);

   private:
    std::map<std::string, std::string> m_config_values;
}
};  // namespace rocket

#endif
