"""Compile and exercise the real CAN implementation with a host GCC and HAL/RTOS stubs."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


HEADERS = {
    "main.h": r"""
#ifndef HOST_MAIN_H
#define HOST_MAIN_H
#include <stdint.h>
typedef struct { unsigned bus; } CAN_HandleTypeDef;
typedef struct { uint32_t StdId, ExtId, IDE, RTR, DLC, TransmitGlobalTime; } CAN_TxHeaderTypeDef;
typedef CAN_TxHeaderTypeDef CAN_RxHeaderTypeDef;
#define CAN_ID_STD 0
#define CAN_RTR_DATA 0
#define CAN_RX_FIFO0 0
#define DISABLE 0
#define HAL_OK 0
uint32_t HAL_GetTick(void);
int HAL_CAN_AddTxMessage(CAN_HandleTypeDef *, CAN_TxHeaderTypeDef *, uint8_t *, uint32_t *);
int HAL_CAN_GetRxMessage(CAN_HandleTypeDef *, uint32_t, CAN_RxHeaderTypeDef *, uint8_t *);
#endif
""",
    "FreeRTOS.h": r"""
#include <stddef.h>
#include <stdint.h>
#define pdPASS 1
#define portMAX_DELAY UINT32_MAX
#define taskENTER_CRITICAL() ((void)0)
#define taskEXIT_CRITICAL() ((void)0)
""",
    "queue.h": r"""
typedef void *QueueHandle_t;
QueueHandle_t xQueueCreate(unsigned, unsigned);
int xQueueSend(QueueHandle_t, const void *, unsigned);
int xQueueReceive(QueueHandle_t, void *, unsigned);
""",
    "cmsis_os.h": "#include <stdint.h>\nvoid osDelay(uint32_t);\n",
    "task.h": "",
    "detect_task.h": "#define CHASSIS_MOTOR1_TOE 0\nvoid detect_hook(unsigned);\n",
}

HARNESS = r"""
#include "main.h"
#include "CAN_receive.h"
#include <assert.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

CAN_HandleTypeDef hcan1 = {1}, hcan2 = {2};
static unsigned item_size, queue_count, head, tail;
static uint8_t queue_storage[32][128];
static jmp_buf drained;
static unsigned sent_count, attempts, failures, delays;
static uint8_t sent[64][8], rx_data[8];
static CAN_TxHeaderTypeDef headers[64];
static unsigned buses[64];
static CAN_RxHeaderTypeDef rx_header;
static uint32_t tick;

void *xQueueCreate(unsigned length, unsigned size) {
    assert(length == 32 && size <= 128);
    item_size = size;
    return queue_storage;
}
int xQueueSend(void *queue, const void *item, unsigned wait) {
    (void)queue; assert(wait == 0);
    if (queue_count == 32) return 0;
    memcpy(queue_storage[head], item, item_size);
    head = (head + 1) % 32; queue_count++;
    return 1;
}
int xQueueReceive(void *queue, void *item, unsigned wait) {
    (void)queue; (void)wait;
    if (!queue_count) longjmp(drained, 1);
    memcpy(item, queue_storage[tail], item_size);
    tail = (tail + 1) % 32; queue_count--;
    return 1;
}
uint32_t HAL_GetTick(void) { return tick; }
void osDelay(uint32_t delay) { delays += delay; }
void detect_hook(unsigned index) { (void)index; }
int HAL_CAN_AddTxMessage(CAN_HandleTypeDef *bus, CAN_TxHeaderTypeDef *header,
                        uint8_t *data, uint32_t *mailbox) {
    (void)mailbox; attempts++;
    if (failures) { failures--; return 1; }
    assert(sent_count < 64);
    buses[sent_count] = bus->bus;
    headers[sent_count] = *header;
    memcpy(sent[sent_count++], data, 8);
    return HAL_OK;
}
int HAL_CAN_GetRxMessage(CAN_HandleTypeDef *bus, uint32_t fifo,
                        CAN_RxHeaderTypeDef *header, uint8_t *data) {
    (void)bus; (void)fifo; *header = rx_header; memcpy(data, rx_data, 8);
    return HAL_OK;
}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *);
static void flush(void) { if (!setjmp(drained)) CAN_tx_task(NULL); }
static void reset(void) {
    assert(!queue_count);
    sent_count = attempts = failures = delays = 0;
}
static void receive(CAN_HandleTypeDef *bus, uint32_t id, const uint8_t *data) {
    rx_header.StdId = id; rx_header.DLC = 8; memcpy(rx_data, data, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(bus);
}
int main(void) {
    CAN_tx_queue_init();
    /* Queuing must copy each local frame and defer HAL access to TX task. */
    CAN_cmd_chassis(-1, 0x1234, -32768, 32767);
    assert(!sent_count); flush();
    const uint8_t motor[] = {255,255,0x12,0x34,0x80,0,0x7f,0xff};
    assert(sent_count == 1 && buses[0] == 1 && headers[0].StdId == 0x200);
    assert(!memcmp(sent[0], motor, 8)); reset();
    CAN_LK_SPEED_Control(1200, -3000);
    CAN_LK_SPEED_Control(600, 3000);
    flush();
    const uint8_t lk1[] = {0xa2,0,0xb0,4,0x48,0xf4,0xff,0xff};
    const uint8_t lk2[] = {0xa2,0,0x58,2,0xb8,0x0b,0,0};
    assert(sent_count == 2 && !memcmp(sent[0],lk1,8) && !memcmp(sent[1],lk2,8));
    reset();
    /* Four failures can recover; five failures drop one frame and continue. */
    failures = 4; CAN_INIT_STATUS(1); flush();
    assert(attempts == 5 && sent_count == 1 && delays == 4); reset();
    uint32_t drops = CAN_get_tx_drop_count();
    failures = 5; CAN_INIT_STATUS(1); CAN_INIT_STATUS(2); flush();
    assert(attempts == 6 && sent_count == 1 && sent[0][0] == 2);
    assert(CAN_get_tx_drop_count() == drops + 1); reset();
    drops = CAN_get_tx_drop_count();
    for (unsigned i=0;i<33;i++) CAN_INIT_STATUS(i);
    flush(); assert(sent_count == 32 && CAN_get_tx_drop_count() == drops + 1);
    for (unsigned i=0;i<32;i++) assert(sent[i][0] == i);
    reset();
    /* New referee frame retains bullet fields and adds big-endian power limit. */
    CAN_cmd_referee_data(27, 22.5f, 0x1234); flush();
    const uint8_t referee[] = {27,0,0,0xb4,0x41,0x12,0x34,0};
    assert(headers[0].StdId == 5 && buses[0] == 2 && !memcmp(sent[0],referee,8));
    reset();
    super_power_t control = {0};
    control.enableDCDC = control.enableActiveChargingLimit = control.useNewFeedbackMessage = 1;
    control.refereePowerLimit = 100; control.refereeEnergyBuffer = 60;
    control.activeChargingLimitRatio = 100;
    CAN_SuperPower_Control(control); flush();
    const uint8_t power[] = {0xc1,100,0,60,0,100,0,0};
    assert(headers[0].StdId == 0x61 && buses[0] == 1 && !memcmp(sent[0],power,8));
    reset();
    /* New incoming targets and super capacitor feedback/timeout. */
    float targets[] = {12.5f,-30.0f};
    receive(&hcan2,0x06,(const uint8_t *)targets);
    assert(get_Chassisdata_point()->yaw_vel == 12.5f);
    assert(get_Chassisdata_point()->yaw_acc == -30.0f);
    const uint8_t feedback[] = {3,0,0x59,0,0x54,100,0,125};
    tick=100; receive(&hcan1,0x52,feedback);
    super_power_receive_t snapshot;
    CAN_get_super_power_data(&snapshot);
    assert(snapshot.statusCode==3 && snapshot.chassisPower==100.0f);
    assert(snapshot.refereePower==80.0f && snapshot.chassisPowerLimit==100);
    assert(snapshot.capEnergy==50 && snapshot.updateTick==100);
    tick=199; assert(CAN_super_power_is_online());
    tick=200; assert(!CAN_super_power_is_online());
    puts("PASS: CAN frame copies, bus routing, retries, overflow, referee extension, super capacitor TX/RX, yaw derivatives, timeout");
    return 0;
}
"""


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--cc", default=os.environ.get("CC", "gcc"))
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="omni-can-") as directory:
        work = Path(directory)
        source = Path(__file__).resolve().parents[1] / "src/application"
        for name in ("CAN_receive.c", "CAN_receive.h", "struct_typedef.h"):
            shutil.copyfile(source / name, work / name)
        for name, content in HEADERS.items():
            (work / name).write_text(content, encoding="utf-8")
        (work / "host.c").write_text(HARNESS, encoding="utf-8")
        executable = work / ("test.exe" if os.name == "nt" else "test")
        subprocess.run([args.cc, "-std=c11", "-O0", "-Werror=implicit-function-declaration",
                        str(work / "CAN_receive.c"), str(work / "host.c"),
                        "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
