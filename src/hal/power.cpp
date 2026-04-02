#include "hal.h"
#include <Arduino.h>
#include "config.h"
#include <driver/rtc_io.h>

// 电池电压滤波
#define BATTERY_SAMPLES 10
static float battery_voltage_buffer[BATTERY_SAMPLES];
static int battery_sample_index = 0;
static bool battery_filter_initialized = false;

// 电压校准系数（根据实际硬件测量调整）
// 如果电压读数偏高，减小此值；如果偏低，增大此值
#define VOLTAGE_CALIBRATION_FACTOR  1.0f

void HAL::power_init(void)
{
    pinMode(BATTERY_OFF_PIN, OUTPUT);
    digitalWrite(BATTERY_OFF_PIN, LOW);  //

    pinMode(ON_OFF_PIN, OUTPUT);
    digitalWrite(ON_OFF_PIN, HIGH);

    // 配置 ADC 引脚
    pinMode(BATTERY_ADC_PIN, ANALOG);
    analogSetAttenuation(ADC_11db);  // 设置 ADC 衰减为 11dB，测量范围 0-3.3V
    analogReadResolution(12);        // 设置ADC分辨率为12位（默认值）

    // 电压校准说明：
    // 1. 用万用表测量实际电池电压
    // 2. 查看串口输出的 "ADC: xxx, Raw: x.xxV, Filtered: x.xxV"
    // 3. 计算校准系数：实际电压 / 滤波后电压
    // 4. 修改上面的 VOLTAGE_CALIBRATION_FACTOR 值
    //
    // 例如：实际电压3.7V，显示电压4.5V，则校准系数 = 3.7 / 4.5 = 0.82

    }

void HAL::power_off(void)
{
    /*
     * ___-__----..
     * MT3608: a high input at EN turns on the conveter
     * low input turns it off
    */
    digitalWrite(BATTERY_OFF_PIN, HIGH);
    delay(100);
    digitalWrite(BATTERY_OFF_PIN, LOW);
    delay(200);
    digitalWrite(BATTERY_OFF_PIN, HIGH);

    // keep PUSH_PIN HIGH level in deep sleep mode
    rtc_gpio_init((gpio_num_t)PUSH_BUTTON_PIN);
    rtc_gpio_pullup_en((gpio_num_t)PUSH_BUTTON_PIN);
    rtc_gpio_pulldown_dis((gpio_num_t)PUSH_BUTTON_PIN);
    gpio_deep_sleep_hold_en();
    // low level will trigger wakeup
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PUSH_BUTTON_PIN, 0);
    esp_deep_sleep_start();
}

float HAL::hal_read_battery_voltage(void)
{
    // 使用 Arduino analogRead 读取 ADC
    // Arduino analogRead 默认返回 0-4095 对应 0-3.3V (12位精度)
    int adc_value = analogRead(BATTERY_ADC_PIN);

    // 转换为电压值
    // ESP32-S3 ADC默认是12位（0-4095），对应0-3.3V
    // 假设电池通过分压电路连接，分压比需要根据实际硬件调整
    // 当前设置：ADC衰减为11dB，测量范围0-3.3V
    float voltage = (adc_value / 4095.0f) * 3.3f;

    // 根据实际硬件分压比调整（需要根据实际电路修改）
    // 假设分压比为1:2（实际电压 = ADC电压 * 2）
    voltage = voltage * 2.0f;

    // 应用校准系数
    voltage = voltage * VOLTAGE_CALIBRATION_FACTOR;

    // 初始化滤波缓冲区
    if (!battery_filter_initialized) {
        for (int i = 0; i < BATTERY_SAMPLES; i++) {
            battery_voltage_buffer[i] = voltage;
        }
        battery_filter_initialized = true;
    }

    // 移动平均滤波
    battery_voltage_buffer[battery_sample_index] = voltage;
    battery_sample_index = (battery_sample_index + 1) % BATTERY_SAMPLES;

    float filtered_voltage = 0;
    for (int i = 0; i < BATTERY_SAMPLES; i++) {
        filtered_voltage += battery_voltage_buffer[i];
    }
    filtered_voltage /= BATTERY_SAMPLES;

    // 打印原始值和滤波后的值
    static int print_counter = 0;
    if (++print_counter >= 60) {  // 每60次打印一次（约1分钟）
        log_i("[Battery] ADC: %d, Raw: %.2fV, Filtered: %.2fV", adc_value, voltage, filtered_voltage);
        print_counter = 0;
    }

    return filtered_voltage;
}

int HAL::hal_read_battery_percentage(void)
{
    float voltage = HAL::hal_read_battery_voltage();
    int percentage = 0;

    // 锂电池电压范围（单节锂电池）
    // 3.0V (0%) - 4.2V (100%)
    // 如果使用多节电池或不同类型电池，需要调整这些值
    float min_voltage = 3.0f;
    float max_voltage = 4.2f;

    // 限制电压范围
    if (voltage <= min_voltage) {
        percentage = 0;
    } else if (voltage >= max_voltage) {
        percentage = 100;
    } else {
        // 线性映射
        percentage = (int)((voltage - min_voltage) / (max_voltage - min_voltage) * 100);
    }

    // 确保百分比在0-100范围内
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    return percentage;
}

bool HAL::hal_battery_is_charging(void)
{
    // 检测 ON_OFF_PIN 是否为高电平（表示充电中）
    // 这个逻辑需要根据实际硬件电路调整
    return digitalRead(ON_OFF_PIN) == HIGH;
}

void HAL::power_update(void)
{
    // 这里可以添加电量更新逻辑
    // 通常在主循环中调用
}

