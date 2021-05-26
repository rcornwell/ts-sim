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
class Device
{
public:

    
    virtual init();
    virtual shutdown();
    virtual start();
    virtual reset();
    virtual stop();
    virtual step();
    virtual run();
    virtual examine();
    virtual deposit();

private:
}
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
        virtual emulator::Device *create() { \
            return new emulator::systype##_##type(); \
        } \
    }; \
    static model##DeviceFactory global_##type##DeviceFactory; \
    };
    


namespace core {

class DeviceFactory
{
public:
    virtual emulator::Device *create() = 0;
};

}