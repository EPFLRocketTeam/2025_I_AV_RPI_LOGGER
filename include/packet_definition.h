#pragma once
#include <cstdint>
#include <sstream>
#include <string>

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
    uint32_t timestamp;
};
#pragma pack(pop)

const size_t log_packet_size = sizeof(log_packet_t);


// inline std::string packetToCSV(const log_packet_t &p)
// {
//     std::ostringstream ss;
//     ss  << fixed16_to_float(p.gyro_x) << "," << fixed16_to_float(p.gyro_y) << "," << fixed16_to_float(p.gyro_z) << ","
//         << fixed16_to_float(p.acc_x) << "," << fixed16_to_float(p.acc_y) << "," << fixed16_to_float(p.acc_z) << ","
//         << fixed16_to_float(p.baro) << ","
//         << fixed16_to_float(p.kalman_yaw) << "," << fixed16_to_float(p.kalman_pitch) << "," << fixed16_to_float(p.kalman_roll) << ","
//         << fixed16_to_float(p.gimbal_x) << "," << fixed16_to_float(p.gimbal_y) << ","
//         << fixed16_to_float(p.hv_voltage) << "," << fixed16_to_float(p.lv_voltage) << ","
//         << fixed16_to_float(p.chamber_pressure) << "," << fixed16_to_float(p.pressure_ETH) << "," << fixed16_to_float(p.pressure_N2O) << ","
//         << fixed16_to_float(p.pressure_inj_ETH) << "," << fixed16_to_float(p.pressure_inj_N2O) << "," << fixed16_to_float(p.temp_N2O) << ","
//         << (bool)p.vent_ETH << "," << (bool)p.vent_N2O << "," << (bool)p.sol_N2 << ","
//         << (int)p.main_ETH << "," << (int)p.main_N2O << ","
//         << (bool)p.sol_ETH << "," << (bool)p.sol_N2O << ","
//         << (bool)p.igniter << ","
//         << (bool)p.sequence_finished << ","
//         << (bool)p.main_valves_homing << "," << (bool)p.main_valves_homing_done << ","
//         << (bool)p.gimbal_homing << "," << (bool)p.gimbal_homing_done << ","
//         << (bool)p.cmd_idle << "," << (bool)p.cmd_arm << "," << (bool)p.cmd_launch << ","
//         << (bool)p.cmd_abort << "," << (bool)p.cmd_tare_orientation << "," << (bool)p.cmd_tare_pressures << ","
//         << (int)p.hopper_state 
//         << (uint32_t)p.timestamp;
//     return ss.str();
// }

