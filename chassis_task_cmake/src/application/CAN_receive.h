/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       can_receive.c/h
  * @brief      there is CAN interrupt function  to receive motor data,
  *             and CAN send function to send motor current to control motor.
  *             ������CAN�жϽ��պ��������յ������,CAN���ͺ������͵���������Ƶ��.
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. done
  *  V1.1.0     Nov-11-2019     RM              1. support hal lib
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#ifndef CAN_RECEIVE_H
#define CAN_RECEIVE_H

#include "struct_typedef.h"

#define CHASSIS_CAN hcan1
#define GIMBAL_CAN hcan1
#define REFEREE_CAN hcan2
/* CAN send and receive ID */
typedef enum
{
  CAN_CHASSIS_ALL_ID = 0x200,
  CAN_3508_M1_ID = 0x201,
  CAN_3508_M2_ID = 0x202,
  CAN_3508_M3_ID = 0x203,
  CAN_3508_M4_ID = 0x204,

  CAN_YAW_MOTOR_ID = 0x205,
  CAN_PIT_MOTOR_ID = 0x206,
  CAN_TRIGGER_MOTOR_ID = 0x207,
  CAN_GIMBAL_ALL_ID = 0x1FF,

  CAN_gmbial_chassis_data1 = 0x01,
  CAN_gmbial_chassis_data2 = 0x02,
  CAN_gmbial_chassis_data3 = 0x03,
  CAN_gmbial_chassis_data4 = 0x04,
  CAN_chassis_gimbal_ID = 0x123,
  CAN_referee_data = 0x05,

  CAN_LK_MOTOR_M1_ID = 0x141,
  /* ��������ID */
  CAN_SUPER_CAP_ID = 0x211,
  /* ������������ID */
  CAN_SUPER_CAP_SET_ID = 0x210,

} can_msg_id_e;

//rm motor data
typedef struct
{
    uint16_t ecd;
    int16_t speed_rpm;
    int16_t given_current;
    uint8_t temperate;
    int16_t last_ecd;
	fp32 angle;
    int32_t ecd_count;
} motor_measure_t;
typedef struct
{
	int8_t temp;
	int16_t iq;
	int16_t speed;
	uint16_t encoder;
	int16_t last_encoder;
	float angle;
}lkmotor_measure_t;
//extern lkmotor_measure_t;
typedef union
{
	float float_t;
	uint8_t uint8_t[4];
} send_float_typedef;
typedef enum
{
  CHASSIS_MODE_OFF=0,
  CHASSIS_MODE_NO_FOLLOW,
  CHASSIS_MODE_FOLLOW,
  CHASSIS_MODE_INIT,
  CHASSIS_MODE_L_TOP,
	CHASSIS_MODE_R_TOP
}RC_chassis_mode_e;

typedef enum
{
    SHOOT_STOP = 0,   
    SHOOT_READY_FRIC,  
    SHOOT_READY_BULLET,


    SHOOT_SINGLE,
    SHOOT_CONTINUE,
    SHOOT_READY,       
    SHOOT_BULLET,      
    SHOOT_CONTINUE_BULLET,
    SHOOT_DONE,     
    SHOOT_BACK,       
} shoot_mode_e;

typedef struct
{
	float vx_set;//����x�᷽���趨���ٶȿ�������
	float vy_set;//����y�᷽���趨���ٶȿ�����
	float wz_set;//��������ʱ �趨���ٶȿ�������
	float yaw_angle;//yaw��ʵʱ�Ƕ�
	float yaw_angle_set;//����yaw��Ƕ�
	float yaw_gyro;//yaw����ٶ�
	RC_chassis_mode_e chassis_mode;//����ģʽ
	shoot_mode_e shoot_mode_rc;//���յ������ģʽ
  uint16_t shoot_mode_receive;
}chassis_data_t;	


/**
  * @brief          send control current of motor (0x205, 0x206, 0x207, 0x208)
  * @param[in]      yaw: (0x205) 6020 motor control current, range [-30000,30000] 
  * @param[in]      pitch: (0x206) 6020 motor control current, range [-30000,30000]
  * @param[in]      shoot: (0x207) 2006 motor control current, range [-10000,10000]
  * @param[in]      rev: (0x208) reserve motor control current
  * @retval         none
  */
/**
  * @brief          ���͵�����Ƶ���(0x205,0x206,0x207,0x208)
  * @param[in]      yaw: (0x205) 6020������Ƶ���, ��Χ [-30000,30000]
  * @param[in]      pitch: (0x206) 6020������Ƶ���, ��Χ [-30000,30000]
  * @param[in]      shoot: (0x207) 2006������Ƶ���, ��Χ [-10000,10000]
  * @param[in]      rev: (0x208) ������������Ƶ���
  * @retval         none
  */
extern void CAN_cmd_gimbal(int16_t yaw, int16_t pitch, int16_t shoot, int16_t rev);

/**
  * @brief          send CAN packet of ID 0x700, it will set chassis motor 3508 to quick ID setting
  * @param[in]      none
  * @retval         none
  */
/**
  * @brief          ����IDΪ0x700��CAN��,��������3508��������������ID
  * @param[in]      none
  * @retval         none
  */
extern void CAN_cmd_chassis_reset_ID(void);

/**
  * @brief          send control current of motor (0x201, 0x202, 0x203, 0x204)
  * @param[in]      motor1: (0x201) 3508 motor control current, range [-16384,16384] 
  * @param[in]      motor2: (0x202) 3508 motor control current, range [-16384,16384] 
  * @param[in]      motor3: (0x203) 3508 motor control current, range [-16384,16384] 
  * @param[in]      motor4: (0x204) 3508 motor control current, range [-16384,16384] 
  * @retval         none
  */
/**
  * @brief          ���͵�����Ƶ���(0x201,0x202,0x203,0x204)
  * @param[in]      motor1: (0x201) 3508������Ƶ���, ��Χ [-16384,16384]
  * @param[in]      motor2: (0x202) 3508������Ƶ���, ��Χ [-16384,16384]
  * @param[in]      motor3: (0x203) 3508������Ƶ���, ��Χ [-16384,16384]
  * @param[in]      motor4: (0x204) 3508������Ƶ���, ��Χ [-16384,16384]
  * @retval         none
  */
extern void CAN_cmd_chassis(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4);
extern void CAN_cmd_referee_data(uint8_t bullet_freq, float bullet_speed);
extern void CAN_INIT_STATUS(uint8_t status);
extern void CAN_LK_START_control(void);
extern void CAN_LK_CLOSE_control(void);
extern void CAN_read_lkmotor_state(void);
extern void CAN_LK_POSITION_Control(int32_t angleControl);
extern void CAN_LK_SPEED_Control(int16_t iqControl,int32_t speedControl);

extern void CAN_LK_Torque_Control(int16_t iqControl);
extern void CAN_LK_CLEAR_ERROR_control(void);


extern void CAN_Send_Setpower(uint16_t setPower);

extern uint8_t reset_flag;





/**
  * @brief          return the yaw 6020 motor data point
  * @param[in]      none
  * @retval         motor data point
  */
/**
  * @brief          ����yaw 6020�������ָ��
  * @param[in]      none
  * @retval         �������ָ��
  */
extern const motor_measure_t *get_yaw_gimbal_motor_measure_point(void);
extern const lkmotor_measure_t *get_yaw_gimbal_lkmotor_measure_point(void);

/**
  * @brief          return the pitch 6020 motor data point
  * @param[in]      none
  * @retval         motor data point
  */
/**
  * @brief          ����pitch 6020�������ָ��
  * @param[in]      none
  * @retval         �������ָ��
  */
extern const motor_measure_t *get_pitch_gimbal_motor_measure_point(void);

/**
  * @brief          return the trigger 2006 motor data point
  * @param[in]      none
  * @retval         motor data point
  */
/**
  * @brief          ���ز������ 2006�������ָ��
  * @param[in]      none
  * @retval         �������ָ��
  */
extern const motor_measure_t *get_trigger_motor_measure_point(void);

/**
  * @brief          return the chassis 3508 motor data point
  * @param[in]      i: motor number,range [0,3]
  * @retval         motor data point
  */
/**
  * @brief          ���ص��̵�� 3508�������ָ��
  * @param[in]      i: ������,��Χ[0,3]
  * @retval         �������ָ��
  */
extern const motor_measure_t *get_chassis_motor_measure_point(uint8_t i);
extern chassis_data_t *get_Chassisdata_point(void);

#endif
