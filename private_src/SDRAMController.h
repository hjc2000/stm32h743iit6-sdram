#pragma once
#include <bsp-interface/di/sdram.h>

namespace bsp
{
    /// @brief 封装 FMC 接口。
    class SDRAMController :
        public bsp::sdram::ISDRAMController
    {
    public:
        /// @brief 打开 SDRAM 控制器。
        /// @param timing
        virtual void Open(bsp::sdram::ISDRAMTiming const &timing) override;
    };
} // namespace bsp
