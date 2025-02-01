#ifndef __SATURN_CONFIG_H__
#define __SATURN_CONFIG_H__


#include "log.h"

#include <boost/lexical_cast.hpp>
#include <exception>
#include <memory>
#include <string>
#include <string_view>


namespace saturn {
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

    template<class T>
    class ConfigVar : public ConfigVarBase {
    private:
        T m_value;

    public: 
        using ptr = std::shared_ptr<ConfigVar<T>>;

        ConfigVar(std::string_view name, const T& default_value, std::string_view description) 
        : ConfigVarBase(name, description), m_value(default_value)
        {}

        std::string toString() override {
            return boost::lexical_cast<std::string>(m_value);
        }
        bool fromString(std::string_view str) override {
            try {
                return boost::lexical_cast<T>(str);
            } catch (std::exception& e) {
                // TODO
            }
        }

    };
}

#endif // !__SATURN_CONFIG_H__