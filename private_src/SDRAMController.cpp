#include "SDRAMController.h"
#include <bsp-interface/di/gpio.h>

void bsp::SDRAMController::InitializeGPIO()
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

void bsp::SDRAMController::StartAutoSendingAutoRefreshCommand(bsp::sdram::ISDRAMTiming const &timing)
{
    int refresh_count = timing.T_AutoRefreshCommand_CLK_Count() - 50;
    if (refresh_count < 50)
    {
        throw std::runtime_error{"FMC 的频率过低导致几乎一直都要处于发送自动刷新命令的状态。"};
    }

    HAL_SDRAM_ProgramRefreshRate(&_handle, refresh_count);
}

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

void bsp::SDRAMController::OpenAsReadBurstMode(bsp::sdram::ISDRAMTiming const &timing,
                                               bsp::sdram::property::RowBitCount const &row_bit_count,
                                               bsp::sdram::property::ColumnBitCount const &column_bit_count,
                                               bsp::sdram::property::DataWidth const &data_width,
                                               bsp::sdram::property::ReadBurstLength const &read_burst_length)
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
    InitializeGPIO();

    _handle.Instance = FMC_SDRAM_DEVICE;
    _handle.Init.SDBank = FMC_SDRAM_BANK1;
    _handle.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
    _handle.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
    _handle.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
    _handle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    _handle.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
    _handle.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    _handle.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    _handle.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    _handle.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_2;

    FMC_SDRAM_TimingTypeDef timing_def{};
    timing_def.LoadToActiveDelay = timing.T_RSC_CLK_Count();
    timing_def.ExitSelfRefreshDelay = timing.T_XSR_CLK_Count();
    timing_def.SelfRefreshTime = timing.T_RAS_CLK_Count();
    timing_def.RowCycleDelay = timing.T_RC_CLK_Count();
    timing_def.WriteRecoveryTime = timing.T_WR_CLK_Count();
    timing_def.RPDelay = timing.T_RP_CLK_Count();
    timing_def.RCDDelay = timing.T_RCD_CLK_Count();

    HAL_SDRAM_Init(&_handle, &timing_def);
    PowerUp();
    StartAutoSendingAutoRefreshCommand(timing);
}

void bsp::SDRAMController::PowerUp()
{
    FMC_SDRAM_CommandTypeDef command{};
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;
    HAL_StatusTypeDef result = HAL_SDRAM_SendCommand(&_handle, &command, 0XFFFF);
    if (result != HAL_StatusTypeDef::HAL_OK)
    {
        throw std::runtime_error{"SDRAM 上电失败。"};
    }
}

void bsp::SDRAMController::PrechargeAll()
{
    FMC_SDRAM_CommandTypeDef command{};
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command.CommandMode = FMC_SDRAM_CMD_PALL;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;
    HAL_StatusTypeDef result = HAL_SDRAM_SendCommand(&_handle, &command, 0XFFFF);
    if (result != HAL_StatusTypeDef::HAL_OK)
    {
        throw std::runtime_error{"发送 “预充电所有 BANK” 命令失败。"};
    }
}

void bsp::SDRAMController::AutoRefresh()
{
    FMC_SDRAM_CommandTypeDef command{};
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;
    HAL_StatusTypeDef result = HAL_SDRAM_SendCommand(&_handle, &command, 0XFFFF);
    if (result != HAL_StatusTypeDef::HAL_OK)
    {
        throw std::runtime_error{"发送 “自动刷新” 命令失败。"};
    }
}

void bsp::SDRAMController::WriteModeRegister(uint32_t value)
{
    FMC_SDRAM_CommandTypeDef command{};
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = value;
    HAL_StatusTypeDef result = HAL_SDRAM_SendCommand(&_handle, &command, 0XFFFF);
    if (result != HAL_StatusTypeDef::HAL_OK)
    {
        throw std::runtime_error{"发送 “写模式寄存器” 命令失败。"};
    }
}
