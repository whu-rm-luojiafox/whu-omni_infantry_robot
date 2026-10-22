#include "main.h"
#include "chassis_task.h"
#include "arm_math.h"
#include "Chassis_power_control.h"
#include "referee.h"
float Klimit=1;
float Plimit=0;
float Watch_Power_Max;
float Watch_Power;
float Watch_Buffer;
float Chassis_pidout_max;
float Chassis_pidout;
float Scaling[4]={1.0,1.0,1.0,1.0};
static uint16_t  Power_Max;
static float Power,Power_Buffer;
int16_t Limit_Chassis_Motor1_Speed_out,Limit_Chassis_Motor2_Speed_out,Limit_Chassis_Motor3_Speed_out,Limit_Chassis_Motor4_Speed_out;
/**
  * @brief  将输入值限制在[min, max]范围内（直接修改原值）
  * @param  goal: 待限制值的指针
  * @param  max: 上限（允许的最大值）
  * @param  min: 下限（允许的最小值）
  */
void Val_Limit(float *goal, float max, float min) {

    // 直接修改原值
    if (*goal > max) {
        *goal = max;
    }else if (*goal < min){
        *goal = min;
    }
}


void chassis_power_limit(chassis_move_t *chassis_move_control)
{
	
	//get_chassis_power_and_buffer();获取功率缓冲值Power_Buffer数据0-60
	get_chassis_power_and_buffer(&Power_Buffer);
	Chassis_pidout_max=64000;//can控制电流值范围为-16384~16384 16000*4 =64000 (保守值)
	Chassis_pidout =
		(fabs(chassis_move_control->motor_speed_pid[0].set-chassis_move_control->motor_speed_pid[0].fdb)+ 
		fabs(chassis_move_control->motor_speed_pid[1].set-chassis_move_control->motor_speed_pid[1].fdb)+
		fabs(chassis_move_control->motor_speed_pid[2].set-chassis_move_control->motor_speed_pid[2].fdb)+
		fabs(chassis_move_control->motor_speed_pid[3].set-chassis_move_control->motor_speed_pid[3].fdb)		);
	
	/*期望滞后占比环-分配单个电机功率-用于上坡分配-联盟赛暂时不使用-如需使用请在定义的时候将Scaling置0，不使用为1*/
	for(uint8_t i=0;i<4;i++)
	{
		Scaling[i]=(chassis_move_control->motor_speed_pid[i].set-chassis_move_control->motor_speed_pid[i].fdb)/Chassis_pidout;	
	}
	
	/*功率满输出占比环-整车底盘总体加速度*/
	/*麦轮3508的线速度解码=电机反馈转速/减速比（19.2032085561）*(轮子周长)3.1415926*15.25/60 线速度 = 电机转子转速/减速比)*π*轮子周长/60(将线速度从米/分钟转换为米/秒)*/
	Klimit=1;//Chassis_pidout/2000;//375*4=1500(需要根据电机来解算-未测量-从can_receive.c中粗略测量)
	Val_Limit(&Klimit,1,-1);
	
	/*缓冲能量占比环-总体约束*/
	if(Power_Buffer<60&&Power_Buffer>=50)		Plimit=0.95;//15
	else if(Power_Buffer<50&&Power_Buffer>=40)	Plimit=0.9;
	else if(Power_Buffer<40&&Power_Buffer>=35)	Plimit=0.75;
	else if(Power_Buffer<35&&Power_Buffer>=30)	Plimit=0.5;
	else if(Power_Buffer<30&&Power_Buffer>=20)	Plimit=0.25;
	else if(Power_Buffer<20&&Power_Buffer>=10)	Plimit=0.125;
	else if(Power_Buffer<10&&Power_Buffer>=0)	Plimit=0.05;
	else if(Power_Buffer==60)					Plimit=1;
	
	// Plimit=0.5f;
	
	// /*同比缩放电流*/
	// Limit_Chassis_Motor1_Speed_out=(int16_t)(Scaling[1]*(chassis_move_control->motor_speed_pid[0].out)*Klimit*Plimit);
	// Limit_Chassis_Motor2_Speed_out=(int16_t)(Scaling[2]*(chassis_move_control->motor_speed_pid[1].out)*Klimit*Plimit);
	// Limit_Chassis_Motor3_Speed_out=(int16_t)(Scaling[3]*(chassis_move_control->motor_speed_pid[2].out)*Klimit*Plimit);
	// Limit_Chassis_Motor4_Speed_out=(int16_t)(Scaling[4]*(chassis_move_control->motor_speed_pid[3].out)*Klimit*Plimit);
}
