#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include <string>
#include "esp_log.h"
#include "servicetags.h"



class ServiceContext {
    constexpr static const char* TAG = "ServiceContext";
    std::unordered_map<std::string, std::shared_ptr<void>> services;

public:

    template <typename Type>
    class Configurator
    {
        std::shared_ptr<Type> ptr;
    public:

        Configurator(std::shared_ptr<Type> ptr) : ptr(ptr) {

        }

        void Config(std::function<void(std::shared_ptr<Type>)> initializer) {
            assert(initializer);
            initializer(ptr);
        }

        //std::shared_ptr<Type> Get(){
        //    return ptr;
        //}

    };

    // Add service without interface using a custom tag
    template <typename Concrete, typename... Args>
    Configurator<Concrete> addService(const char* tag, Args&&... args) {
        ESP_LOGI(TAG, "Adding service '%s'", tag);
        assert(services.find(tag) == services.end() && "Error: Service tag already exists.");
        auto concreteInstance = std::make_shared<Concrete>(std::forward<Args>(args)...);
        assert(concreteInstance);
        services[tag] = std::static_pointer_cast<void>(concreteInstance);
        return Configurator<Concrete>(concreteInstance);
    }

    // Add service with interface and custom tag
    template <typename Interface, typename Concrete, typename... Args>
    Configurator<Concrete> addService(const char* tag, Args&&... args) {
        return addService<Concrete>(tag, std::forward<Args>(args)...);

    }

#ifdef CONFIG_COMPILER_CXX_RTTI
    // Add service without interface and without tag
    template <typename Concrete, typename... Args>
    Configurator<Concrete> addService(Args&&... args) {
        const std::string tag = typeid(Concrete).name();
        return addService<Concrete>(tag.c_str(), std::forward<Args>(args)...);
    }

    // Add service with interface without tag
    template <typename Interface, typename Concrete, typename... Args>
    Configurator<Concrete> addService(Args&&... args) {
        const std::string tag = typeid(Interface).name();
        return addService<Interface, Concrete>(tag.c_str(), std::forward<Args>(args)...);
    }
#endif


    // Get service using custom tag
    template <typename TYPE>
    std::shared_ptr<TYPE> getService(const char* tag) const {
        auto it = services.find(tag);

        if (it == services.end()) {
            ESP_LOGE(TAG, "Error: Service with tag '%s' not found", tag);
            return nullptr;
        }

        return std::static_pointer_cast<TYPE>(it->second);
    }

#ifdef CONFIG_COMPILER_CXX_RTTI
    // Get service without custom tag
    template <typename TYPE>
    std::shared_ptr<TYPE> getService() const {
        const std::string tag = typeid(TYPE).name();
        return getService<TYPE>(tag.c_str());
    }
#endif

};


class Builder {
public:
    ServiceContext Services;

    // Build the application
    template <typename T, typename... Args>
    std::shared_ptr<T> build(Args&&... args) const {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
};
