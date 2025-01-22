#include "SDRAMController.h"
#include <bsp-interface/di/gpio.h>

bsp::SDRAMController &bsp::SDRAMController::Instance()
{
    class Getter :
        public bsp::TaskSingletonGetter<bsp::SDRAMController>
    {
    public:
        std::unique_ptr<bsp::SDRAMController> Create() override
        {
            return std::unique_ptr<bsp::SDRAMController>{new bsp::SDRAMController{}};
        }
    };

    Getter g;
    return g.Instance();
}

void bsp::SDRAMController::Open(bsp::sdram::ISDRAMTiming const &timing)
{
    /* 保护SDRAM区域,共32M字节 */
    mpu_set_protection(0xC0000000,               /* 基地址 */
                       MPU_REGION_SIZE_32MB,     /* 长度 */
                       MPU_REGION_NUMBER6, 0,    /* NUMER6,允许指令访问 */
                       MPU_REGION_FULL_ACCESS,   /* 全访问 */
                       MPU_ACCESS_NOT_SHAREABLE, /* 禁止共用 */
                       MPU_ACCESS_CACHEABLE,     /* 允许cache */
                       MPU_ACCESS_BUFFERABLE);   /* 允许缓冲 */

    __HAL_RCC_FMC_CLK_ENABLE();

    // 初始化 GPIO
    {
        char const *pin_names[] = {
            "PC0", "PC2", "PC3",
            "PD0", "PD1", "PD8", "PD9", "PD10", "PD14", "PD15",
            "PE0", "PE1", "PE7", "PE8", "PE9", "PE10", "PE11", "PE12", "PE13", "PE14", "PE15",
            "PF0", "PF1", "PF2", "PF3", "PF4", "PF5", "PF11", "PF12", "PF13", "PF14", "PF15",
            "PG0", "PG1", "PG2", "PG4", "PG5", "PG8", "PG15"};

        for (char const *pin_name : pin_names)
        {
            bsp::IGpioPin *pin = DI_GpioPinCollection().Get(pin_name);

            pin->OpenAsAlternateFunctionMode("fmc",
                                             bsp::IGpioPinPullMode::PullUp,
                                             bsp::IGpioPinDriver::PushPull);
        }
    }
}
