#ifndef __SATURN_CONFIG_H__
#define __SATURN_CONFIG_H__

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <typeinfo>
#include <type_traits>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>


#include "log.h"
#include "util.h"

namespace saturn {
    template <typename T>
    using config_change_cb = std::function<void (T& old_value, T& new_value)> ;

    class ConfigVarBase {
    protected:
        std::string m_name;
        std::string m_description;
    
    public:
        using ptr = std::shared_ptr<ConfigVarBase>; 
        ConfigVarBase() = delete;
        ConfigVarBase(std::string_view name, std::string_view description = "") 
        : m_name(name), m_description(description) {}
        virtual ~ConfigVarBase();

        std::string_view getName() {return this->m_name;};
        std::string_view getDescription() {return this->m_description;};

        virtual std::string toString() = 0;
        virtual bool fromString(std::string_view str) = 0;
    };

    template<class T
            ,class FromStr = cast<std::string, T>
            ,class ToStr = cast<T, std::string>>
    class ConfigVar : public ConfigVarBase {
    private:
        using cb = config_change_cb<T>;
        T m_value;
        std::map<uint64_t, cb> cb_maps;
    public: 
        using ptr = std::shared_ptr<ConfigVar<T>>;
        ConfigVar(std::string_view name, const T& default_value, std::string_view description) 
        : ConfigVarBase(name, description), m_value(default_value)
        {}

        std::string toString() override {
            try {
                return ToStr()(m_value);
            } catch (std::exception& e) {
                SATURN_LOG_ERROR(LOGGER()) 
                << "error[" << e.what() << "]: cast from "  << typeid(T).name() << " to std::string";
            }
        }
        bool fromString(std::string_view str) override {
            try {
                m_value = FromStr()(str);
                return true;
            } catch (std::exception& e) {
                SATURN_LOG_ERROR(LOGGER()) 
                << "error[" << e.what() << "]: cast from std::string to " << typeid(T).name();
            }
            return false;
        }
        size_t addListener(cb& cb_) {
            uint64_t hash_v = std::hash<cb>();
            cb_maps.emplace(hash_v, cb_);
            return hash_v;
        }

        void delListener(uint64_t hash_v) {
            cb_maps.erase(hash_v);
        }

        cb& getListener(uint64_t hash_v) {
            return cb_maps.contains(hash_v) ? cb_maps[hash_v] : nullptr;
        }

        void clearListener() {
            cb_maps.clear();
        }

    };

    class Config {
        private:
            ConfigVarBase::ptr lookUp(std::string_view name) const {
                if (!m_vars.contains(name.data())) {
                    return nullptr;
                }
                return m_vars[name.data()];
            }

            template<typename T>
            typename ConfigVar<T>::ptr lookUp(std::string_view name) const{
                if (!m_vars.contains(name.data())) {
                    return nullptr;
                }
                return m_vars[name.data()];
            }

            template<typename T>
            typename ConfigVar<T>::ptr add(std::string_view name, const T& default_value, std::string_view description) {
                typename ConfigVar<T>::ptr res = nullptr;
                if ((res = lookUp<T>(name))) {
                    SATURN_LOG_INFO(LOGGER()) << "config var \"" << name << "\" exists";
                    return res;
                }
                res = std::make_shared<T>(name, default_value, description);
                return res;
            }
            void loadFromYaml(const YAML::Node& root);
            

        public:
            using ptr = std::shared_ptr<Config> ;
            static std::map<std::string, ConfigVarBase::ptr> m_vars;

    };
}

#endif // !__SATURN_CONFIG_H__