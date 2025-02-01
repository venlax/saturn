#ifndef __SATURN_CONFIG_H__
#define __SATURN_CONFIG_H__

#include <memory>
#include <string>
#include <string_view>


namespace saturn {
    class ConfigBase {
    protected:
        std::string m_name;
        std::string m_description;
    
    public:
        using ptr = std::shared_ptr<ConfigBase>; 
        ConfigBase() = delete;
        ConfigBase(std::string_view name, std::string_view description = "") 
        : m_name(name), m_description(description) {}
        virtual ~ConfigBase();

        std::string_view getName() {return this->m_name;};
        std::string_view getDescription() {return this->m_description;};

        virtual std::string toString();
        //virtual 
    };

    template<class T>
    class Config : public ConfigBase {
    private:
        T m_value;

    public: 
        using ptr = std::shared_ptr<Config<T>>;

        Config(std::string_view name, const T& default_value, std::string_view description) 
        : ConfigBase(name, description), m_value(default_value)
        {}

        std::string toString() override {

        }
    };
}

#endif // !__SATURN_CONFIG_H__