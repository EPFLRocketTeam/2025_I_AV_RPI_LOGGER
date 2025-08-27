#pragma once
#include <cstdint>
#include <sstream>
#include <string>

static uint16_t float_to_fixed16(float value) 
{
    // Scale by 2^6 (because 6 fractional bits)
    int32_t scaled = (int32_t)roundf(value * (1 << 6));

    // Range check: signed 16-bit fixed Q9.6 covers -32768/64 to 32767/64
    if (scaled > 0x7FFF) {   // clamp to max positive
        scaled = 0x7FFF;
    } else if (scaled < -0x8000) { // clamp to min negative
        scaled = -0x8000;
    }

    // Store as uint16_t, but bit pattern is signed two’s complement
    return (uint16_t)(scaled & 0xFFFF);
}

static float fixed16_to_float(uint16_t fixed) 
{
    // Interpret the bits as a signed 16-bit integer
    int16_t signed_val = (int16_t)fixed;

    // Divide by 2^6 (64) to restore fractional scaling
    return (float)signed_val / (1 << 6);
}

#pragma pack(push, 1)
struct log_packet_t {
    uint16_t gyro_x;
    uint16_t gyro_y;
    uint16_t gyro_z;

    uint16_t acc_x;
    uint16_t acc_y;
    uint16_t acc_z;

    uint16_t baro;

    uint16_t kalman_yaw;
    uint16_t kalman_pitch;
    uint16_t kalman_roll;

    uint16_t gimbal_x;
    uint16_t gimbal_y;

    uint16_t hv_voltage;
    uint16_t lv_voltage;

    uint16_t chamber_pressure;
    uint16_t pressure_ETH;
    uint16_t pressure_N2O;
    uint16_t pressure_inj_ETH;
    uint16_t pressure_inj_N2O;
    uint16_t temp_N2O;

    uint8_t vent_ETH;
    uint8_t vent_N2O;
    uint8_t sol_N2;

    uint8_t main_ETH;
    uint8_t main_N2O;

    uint8_t sol_ETH;
    uint8_t sol_N2O;

    uint8_t igniter;

    uint8_t sequence_finished;

    uint8_t main_valves_homing;
    uint8_t main_valves_homing_done;
    uint8_t gimbal_homing;
    uint8_t gimbal_homing_done;

    uint8_t cmd_idle;
    uint8_t cmd_arm;
    uint8_t cmd_launch;
    uint8_t cmd_abort;
    uint8_t cmd_tare_orientation;
    uint8_t cmd_tare_pressures;

    uint8_t hopper_state;
};
#pragma pack(pop)

const size_t log_packet_size = sizeof(log_packet_t);

inline std::string packetToCSV(const log_packet_t &p) {
    std::ostringstream ss;
    ss  << fixed16_to_float(p.gyro_x) << "," << fixed16_to_float(p.gyro_y) << "," << fixed16_to_float(p.gyro_z) << ","
        << fixed16_to_float(p.acc_x) << "," << fixed16_to_float(p.acc_y) << "," << fixed16_to_float(p.acc_z) << ","
        << fixed16_to_float(p.baro) << ","
        << fixed16_to_float(p.kalman_yaw) << "," << fixed16_to_float(p.kalman_pitch) << "," << fixed16_to_float(p.kalman_roll) << ","
        << fixed16_to_float(p.gimbal_x) << "," << fixed16_to_float(p.gimbal_y) << ","
        << fixed16_to_float(p.hv_voltage) << "," << fixed16_to_float(p.lv_voltage) << ","
        << fixed16_to_float(p.chamber_pressure) << "," << fixed16_to_float(p.pressure_ETH) << "," << fixed16_to_float(p.pressure_N2O) << ","
        << fixed16_to_float(p.pressure_inj_ETH) << "," << fixed16_to_float(p.pressure_inj_N2O) << "," << fixed16_to_float(p.temp_N2O) << ","
        << (bool)p.vent_ETH << "," << (bool)p.vent_N2O << "," << (bool)p.sol_N2 << ","
        << (int)p.main_ETH << "," << (int)p.main_N2O << ","
        << (bool)p.sol_ETH << "," << (bool)p.sol_N2O << ","
        << (bool)p.igniter << ","
        << (bool)p.sequence_finished << ","
        << (bool)p.main_valves_homing << "," << (bool)p.main_valves_homing_done << ","
        << (bool)p.gimbal_homing << "," << (bool)p.gimbal_homing_done << ","
        << (bool)p.cmd_idle << "," << (bool)p.cmd_arm << "," << (bool)p.cmd_launch << ","
        << (bool)p.cmd_abort << "," << (bool)p.cmd_tare_orientation << "," << (bool)p.cmd_tare_pressures << ","
        << (int)p.hopper_state;
    return ss.str();
}

inline std::string csvHeader() {
    return "gyro_x,gyro_y,gyro_z,"
           "acc_x,acc_y,acc_z,"
           "baro,"
           "kalman_yaw,kalman_pitch,kalman_roll,"
           "gimbal_x,gimbal_y,"
           "hv_voltage,lv_voltage,"
           "chamber_pressure,pressure_ETH,pressure_N2O,pressure_inj_ETH,pressure_inj_N2O,temp_N2O,"
           "vent_ETH,vent_N2O,sol_N2,"
           "main_ETH,main_N2O,sol_ETH,sol_N2O,igniter,sequence_finished,"
           "main_valves_homing,main_valves_homing_done,gimbal_homing,gimbal_homing_done,"
           "cmd_idle,cmd_arm,cmd_launch,cmd_abort,cmd_tare_orientation,cmd_tare_pressures,"
           "hopper_state";
}