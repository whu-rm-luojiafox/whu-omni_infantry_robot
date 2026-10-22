#ifndef CHASSIS_POWER_CONTROL_H
#define CHASSIS_POWER_CONTROL_H
#include "struct_typedef.h"
#include "chassis_task.h"
extern int16_t Limit_Chassis_Motor1_Speed_out,Limit_Chassis_Motor2_Speed_out,Limit_Chassis_Motor3_Speed_out,Limit_Chassis_Motor4_Speed_out;
extern float Klimit;
extern float Plimit;
extern float Scaling[4];
void chassis_power_limit(chassis_move_t *chassis_move_control);
#endif

