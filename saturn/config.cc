#include "config.h"

#include <list>
#include <sstream>
#include <utility>


namespace saturn {

    static void iterateNodes(const std::string& prefix,
                            const YAML::Node& root
                            , std::list<std::pair<std::string, const YAML::Node>>& list) {
        list.push_back(std::make_pair(prefix.data(), root));

        if (root.IsMap()) {
            for (auto iter = root.begin(); 
                iter != root.end(); iter++) {
                    iterateNodes(prefix.empty() ? 
                    iter->first.Scalar() :
                    prefix + "." + iter->first.Scalar() 
                    , iter->second , list);
                }
        }
    }
    void Config::loadFromYaml(const YAML::Node& root) {
        std::list<std::pair<std::string, const YAML::Node>> list;
        iterateNodes("", root, list);

        for (auto& iter : list) {
            if (iter.first.empty()) {
                continue;
            }
            ConfigVarBase::ptr var = lookUp(iter.first);
            if (var) {
                if (iter.second.IsScalar()) {
                    var->fromString(iter.second.Scalar());
                } else {
                    std::stringstream ss;
                    ss << iter.second;
                    var->fromString(ss.str());
                }
            }
        }
    }
}