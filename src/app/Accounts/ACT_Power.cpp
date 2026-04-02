#include "Account_Master.h"
#include "hal/hal.h"
#include "lvgl.h"

using namespace AccountSystem;

static HAL::Power_Info_t powerInfo =
{
    .voltage = 0,
    .usage = 0,
    .isCharging = false,
};

static int onEvent(Account* account, Account::EventParam_t* param)
{
    if (param->event == Account::EVENT_SUB_PULL)
    {
        if (param->size != sizeof(HAL::Power_Info_t))
        {
            return Account::ERROR_SIZE_MISMATCH;
        }

        memcpy(param->data_p, &powerInfo, sizeof(powerInfo));
        return 0;
    }

    return Account::ERROR_UNSUPPORTED_REQUEST;
}

// LVGL 定时器更新电量信息
static void battery_update_timer_cb(lv_timer_t* timer)
{
    Account* account = (Account*)timer->user_data;

    // 更新电量信息
    powerInfo.voltage = (uint16_t)(HAL::hal_read_battery_voltage() * 1000);  // 转换为 mV
    powerInfo.usage = (uint8_t)HAL::hal_read_battery_percentage();
    powerInfo.isCharging = HAL::hal_battery_is_charging();

    // 发布更新
    account->Publish();
}

ACCOUNT_INIT_DEF(Power)
{
    account->SetEventCallback(onEvent);

    // 创建定时器，每 1 秒更新一次电量
    lv_timer_create(battery_update_timer_cb, 1000, account);
}