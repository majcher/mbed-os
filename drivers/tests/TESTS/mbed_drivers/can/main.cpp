
/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if !DEVICE_CAN
#error [NOT_SUPPORTED] CAN API not supported for this target
#elifndef CAN1_TD
#error [NOT_SUPPORTED] CAN1_TD pin name not defined for this target
#elifndef CAN1_RD
#error [NOT_SUPPORTED] CAN1_RD pin name not defined for this target
#else

#include "utest/utest.h"
#include "utest/utest_print.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "FlashIAP.h"
#include "unity.h"
#include <algorithm>

#include "mbed.h"

// Debug available
#ifndef CAN_DEBUG
#define CAN_DEBUG      0
#endif

#if CAN_DEBUG
#define DEBUG_PRINTF(...) utest_printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

using namespace utest::v1;

const int WRITE_ATTEMPTS = 5;
const int READ_ATTEMPTS = 5;
const int LOOP_SIZE = 100;
const int ANY_ID = 1337;
const int MSG_LENGTH = 1;
const int BAUD_RATE = 500000;
const int FRAME_SIZE = 1 + 11 + 1 + 1 + 1 + 4 + 8 + 1 + 15 + 2 + 7 + 3;
const CAN::Mode CAN_MODE = CAN::LocalTest;

void can_write_single_test()
{
    CAN can(CAN1_RD, CAN1_TD, BAUD_RATE);
    can.mode(CAN_MODE);

    char msg = 13;
    int ret = can.write(CANMessage(ANY_ID, &msg, MSG_LENGTH));
    DEBUG_PRINTF("CAN write status: %d\n", ret);
    TEST_ASSERT_EQUAL_INT32_MESSAGE(1, ret, "CAN write status");
}

void can_writes_multiple_test()
{
    CAN can(CAN1_RD, CAN1_TD, BAUD_RATE);
    can.mode(CAN_MODE);

    char counter = 0;
    while (counter < LOOP_SIZE) {
        int ret;
        // write with retries to allow the Tx FIFO queue to drain
        for (int i=0; i<WRITE_ATTEMPTS; i++) {
            ret = can.write(CANMessage(ANY_ID, &counter, MSG_LENGTH));
            // ret is 0 if write failed, 1 if write was successful
            if (ret) break;
            wait_us(FRAME_SIZE * 1000*1000/BAUD_RATE);
        }
        DEBUG_PRINTF("CAN write status: %d\n", ret);
        TEST_ASSERT_EQUAL_INT32_MESSAGE(1, ret, "CAN write status");
        counter++;
    }
}

void can_read_test()
{
    CAN can(CAN1_RD, CAN1_TD, BAUD_RATE);
    can.mode(CAN_MODE);

    char counter = 0;
    while (counter < LOOP_SIZE) {
        int ret;
        for (int i=0; i<WRITE_ATTEMPTS; i++) {
            ret = can.write(CANMessage(ANY_ID, &counter, MSG_LENGTH));
            // ret is 0 if write failed, 1 if write was successful
            if (ret) break;
            wait_us(FRAME_SIZE * 1000*1000/BAUD_RATE);
        }
        // ret is 0 if write failed, 1 if write was successful
        DEBUG_PRINTF("CAN write status: %d\n", ret);
        TEST_ASSERT_EQUAL_INT32_MESSAGE(1, ret, "CAN write status");

        CANMessage msg;
        for (int i=0; i<READ_ATTEMPTS; i++) {
            ret = can.read(msg);
            if (ret) break;
            wait_us(FRAME_SIZE * 1000*1000/BAUD_RATE);
        }
        DEBUG_PRINTF("CAN read status: %d\n", ret);
        TEST_ASSERT_EQUAL_INT32_MESSAGE(1, ret, "CAN read status");

        DEBUG_PRINTF("CAN read message: %d, expected: %d\n", msg.data[0], counter);
        TEST_ASSERT_EQUAL_INT32_MESSAGE(counter, msg.data[0], "Message content");

        counter++;
    }
}


Case cases[] = {
//    Case("CAN API - write single", can_write_single_test),
//    Case("CAN API - write multiple", can_writes_multiple_test),
    Case("CAN API - read", can_read_test)
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases)
{
    GREENTEA_SETUP(120, "default_auto");
    return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main()
{
    Harness::run(specification);
}

#endif // !DEVICE_CAN
