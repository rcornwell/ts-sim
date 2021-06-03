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

    virtual void setAddress(size_t addr)
    {
	    addr_ = addr;
    }

    virtual size_t getAddress() const
    {
	    return addr_;
    }

    virtual size_t getSize() const
    {
	    return 1;
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
    
    virtual bool input(T &val, [[maybe_unused]]size_t port)
    {
        val = 0;
        return false;
    }
    virtual bool output([[maybe_unused]]T val, [[maybe_unused]]size_t port)
    {
        return false;
    }
    
    virtual
    core::ConfigOptionParser options()
    {
        core::ConfigOptionParser option("Device Options");
        return option;
    }

protected:
    size_t addr_;
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
