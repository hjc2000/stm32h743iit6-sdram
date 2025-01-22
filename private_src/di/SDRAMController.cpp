#include <bsp-interface/di/sdram.h>
#include <SDRAMController.h>

bsp::sdram::ISDRAMController &bsp::di::sdram::SDRAMController()
{
    return bsp::SDRAMController::Instance();
}
