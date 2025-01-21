#pragma once
#include <bsp-interface/di/sdram.h>

namespace bsp
{
    /// @brief 封装 FMC 接口。
    class SDRAMController :
        public bsp::ISDRAMController
    {
    public:
        /// @brief 打开 SDRAM 控制器。
        /// @param timing
        virtual void Open(bsp::ISDRAMTiming const &timing) = 0;

        /// @brief 此 SDRAM 控制器所管理的内存段。
        /// @return
        virtual base::Span Span() const = 0;
    };
} // namespace bsp
