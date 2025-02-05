#include "config.h"


#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

#include "log.h"

using namespace saturn;

ConfigVar<int>::ptr config = Config::add("int",5, "okk");


int main() {
    SATURN_LOG_INFO(LOGGER()) << "value" <<  config->getValue();

    YAML::Node node = YAML::LoadFile("/home/venicebitch/saturn/tests/config/int.yaml");

    // std::cout << node << std::endl;

    Config::loadFromYaml(node);
    
    SATURN_LOG_INFO(LOGGER()) << "value" << config->getValue();

}