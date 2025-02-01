#ifndef __SATURN_CONFIG_H__
#define __SATURN_CONFIG_H__

#include <boost/lexical_cast.hpp>
#include <memory>
#include <string>
#include <string_view>


namespace saturn {
    class ConfigValBase {
    protected:
        std::string m_name;
        std::string m_description;
    
    public:
        using ptr = std::shared_ptr<ConfigValBase>; 
        ConfigValBase() = delete;
        ConfigValBase(std::string_view name, std::string_view description = "") 
        : m_name(name), m_description(description) {}
        virtual ~ConfigValBase();

        std::string_view getName() {return this->m_name;};
        std::string_view getDescription() {return this->m_description;};

        virtual std::string toString() = 0;
        virtual bool fromString(std::string_view str) = 0;
    };

    template<class T>
    class ConfigVal : public ConfigValBase {
    private:
        T m_value;

    public: 
        using ptr = std::shared_ptr<ConfigVal<T>>;

        ConfigVal(std::string_view name, const T& default_value, std::string_view description) 
        : ConfigValBase(name, description), m_value(default_value)
        {}

        std::string toString() override {
            return boost::lexical_cast<std::string>(m_value);
        }
    };
}

#endif // !__SATURN_CONFIG_H__