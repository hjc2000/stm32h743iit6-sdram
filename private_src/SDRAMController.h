#pragma once
#include <base/define.h>
#include <bsp-interface/di/sdram.h>
#include <bsp-interface/TaskSingletonGetter.h>
#include <hal.h>

namespace bsp
{
    /// @brief 封装 FMC 接口。
    class SDRAMController :
        public bsp::sdram::ISDRAMController
    {
    private:
        SDRAMController() = default;

        SDRAM_HandleTypeDef _handle{};

        uint8_t mpu_set_protection(uint32_t baseaddr,
                                   uint32_t size,
                                   uint32_t rnum,
                                   uint8_t de,
                                   uint8_t ap,
                                   uint8_t sen,
                                   uint8_t cen,
                                   uint8_t ben)
        {
            MPU_Region_InitTypeDef mpu_region_init_handle{};

            HAL_MPU_Disable(); /* 配置MPU之前先关闭MPU,配置完成以后在使能MPU */

            mpu_region_init_handle.Enable = MPU_REGION_ENABLE;     /* 使能该保护区域 */
            mpu_region_init_handle.Number = rnum;                  /* 设置保护区域 */
            mpu_region_init_handle.BaseAddress = baseaddr;         /* 设置基址 */
            mpu_region_init_handle.DisableExec = de;               /* 是否允许指令访问 */
            mpu_region_init_handle.Size = size;                    /* 设置保护区域大小 */
            mpu_region_init_handle.SubRegionDisable = 0X00;        /* 禁止子区域 */
            mpu_region_init_handle.TypeExtField = MPU_TEX_LEVEL0;  /* 设置类型扩展域为level0 */
            mpu_region_init_handle.AccessPermission = (uint8_t)ap; /* 设置访问权限, */
            mpu_region_init_handle.IsShareable = sen;              /* 是否共用? */
            mpu_region_init_handle.IsCacheable = cen;              /* 是否cache? */
            mpu_region_init_handle.IsBufferable = ben;             /* 是否缓冲? */
            HAL_MPU_ConfigRegion(&mpu_region_init_handle);         /* 配置MPU */
            HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);                /* 开启MPU */
            return 0;
        }

    public:
        static_function SDRAMController &Instance();

        /// @brief 打开 SDRAM 控制器。
        /// @param timing
        virtual void Open(bsp::sdram::ISDRAMTiming const &timing) override;
    };
} // namespace bsp
