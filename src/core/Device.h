#pragma once

namespace emulator
{

/**
 * @class Device
 * @author rich
 * @date 21/03/21
 * @file Device.h
 * @brief
 */
template <typename T>
class Device
{
public:
    Device()
    {
    }
    
    virtual ~Device()
    {
    }
    
    virtual void init() {}
    virtual void shutdown() {}
    virtual void start() {}
    virtual void reset() {}
    virtual void stop() {}
    virtual void step() {}
    virtual void run() {}
    virtual void examine() {}
    virtual void deposit() {}
    
    virtual bool input(T &val, size_t port)
    {
        val = 0;
        return false;
    }
    virtual bool output(T val, size_t port)
    {
        return false;
    }
    
    virtual
    core::ConfigOptionParser options()
    {
        core::ConfigOptionParser option("Device Options");
        return option;
    }


private:
};
}

#define REGISTER_DEVICE(systype, type) \
    namespace core { \
    class model##DeviceFactory : public DeviceFactory { \
    public: \
        model##DeviceFactory() \
        { \
            std::cout << "Registering Device: " #type << "\n"; \
            systype::registerDevice(#type, this); \
        } \
        virtual DEV_v create() { \
            return std::make_shared<emulator::##systype##_##type>();  \
        } \
    }; \
    static model##DeviceFactory global_##type##DeviceFactory; \
    };
    

using DEV_v = std::variant<std::shared_ptr<emulator::Device<uint8_t>>,
      std::shared_ptr<emulator::Device<uint16_t>>,
      std::shared_ptr<emulator::Device<uint32_t>>,
      std::shared_ptr<emulator::Device<uint64_t>>>;

namespace core {

class DeviceFactory
{
public:
    virtual DEV_v *create() = 0;
};

}