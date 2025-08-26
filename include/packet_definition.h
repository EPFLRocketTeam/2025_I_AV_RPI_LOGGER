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
};
#pragma pack(pop)

inline std::string packetToCSV(const log_packet &p) {
    std::ostringstream ss;
    ss  << p.gyro_x << "," << p.gyro_y << "," << p.gyro_z << ","
        << p.acc_x << "," << p.acc_y << "," << p.acc_z << ","
        << p.baro << ","
        << p.kalman_yaw << "," << p.kalman_pitch << "," << p.kalman_roll << ","
        << p.gimbal_x << "," << p.gimbal_y << ","
        << p.hv_voltage << "," << p.lv_voltage << ","
        << p.chamber_pressure << "," << p.pressure_ETH << "," << p.pressure_N2O << ","
        << p.pressure_inj_ETH << "," << p.pressure_inj_N2O << "," << p.temp_N2O << ","
        << (int)p.vent_ETH << "," << (int)p.vent_N2O << "," << (int)p.sol_N2 << ","
        << (int)p.main_ETH << "," << (int)p.main_N2O << ","
        << (int)p.sol_ETH << "," << (int)p.sol_N2O << ","
        << (int)p.igniter << ","
        << (int)p.sequence_finished << ","
        << (int)p.main_valves_homing << "," << (int)p.main_valves_homing_done << ","
        << (int)p.gimbal_homing << "," << (int)p.gimbal_homing_done << ","
        << (int)p.cmd_idle << "," << (int)p.cmd_arm << "," << (int)p.cmd_launch << ","
        << (int)p.cmd_abort << "," << (int)p.cmd_tare_orientation << "," << (int)p.cmd_tare_pressures << ","
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