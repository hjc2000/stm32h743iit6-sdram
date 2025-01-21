#pragma once
#include <base/define.h>
#include <bsp-interface/di/sdram.h>
#include <bsp-interface/TaskSingletonGetter.h>

namespace bsp
{
    /// @brief 封装 FMC 接口。
    class SDRAMController :
        public bsp::sdram::ISDRAMController
    {
    private:
        SDRAMController() = default;

    public:
        static_function SDRAMController &Instance();

        /// @brief 打开 SDRAM 控制器。
        /// @param timing
        virtual void Open(bsp::sdram::ISDRAMTiming const &timing) override;
    };
} // namespace bsp
