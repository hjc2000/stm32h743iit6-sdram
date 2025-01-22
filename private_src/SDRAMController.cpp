#include "SDRAMController.h"

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
}
