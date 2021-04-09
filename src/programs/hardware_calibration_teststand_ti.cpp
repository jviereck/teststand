/**
 * @file teststand_hardware_calibration.cpp
 * @author Maximilien Naveau (maximilien.naveau@gmail.com)
 * @brief Program that launch the calibration of the testand with 0 offset
 * @version 0.1
 * @date 2019-11-08
 *
 * @copyright Copyright (c) 2019
 *
 * This program is a helper for the measurement of the offset between the joint
 * index (home pose) and the theoretical zero defined by the urdf.
 *
 * - It start by initializing the robot.
 * - Once done it wait for the user to press enter in order to move the leg to
 *   the joint index
 * - Then the controller just sends 0 torques and report the current joint pose.
 *
 * Once this small procedure is done the user can setup the joints in the zero
 * configuration and read the offset in the screen. The offset should be
 * reported at least in the
 * robot_properties_[robot_name]/config/dgm_paramter.yaml under the
 * ```
 * hardware_communication:
 *   calibration:
 *      index_to_zero_angle: [...]
 * ```
 */

/**
 * \file demo_test_bench_8_motors.cpp
 * \brief The use of the wrapper implementing a small pid controller.
 * \author Maximilien Naveau
 * \date 2018
 *
 * This file uses the Teststand class in a small demo.
 */


#include "teststand/utils.hpp"
#include "teststand/teststand_ti.hpp"

using namespace teststand;

static THREAD_FUNCTION_RETURN_TYPE control_loop(void* robot_void_ptr)
{
    TeststandTi& robot = *(static_cast<TeststandTi*>(robot_void_ptr));

    Eigen::Vector2d joint_index_to_zero;
    joint_index_to_zero.setZero();
    robot.calibrate(joint_index_to_zero);

    long int count = 0;
    while (!CTRL_C_DETECTED)
    {
        if (count % 200 == 0)
        {
            robot.acquire_sensors();
            print_vector("Joint Positions", robot.get_joint_positions());
        }
    }

    CTRL_C_DETECTED = true;

    return THREAD_FUNCTION_RETURN_VALUE;
}  // end control_loop

int main(int, char**)
{
    enable_ctrl_c();

    real_time_tools::RealTimeThread thread;

    TeststandTi robot;

    robot.initialize();

    rt_printf("controller is set up \n");
    rt_printf("Press enter to launch the calibration \n");
    char str[256];
    std::cin.get(str, 256);  // get c-string

    thread.create_realtime_thread(&control_loop, &robot);

    // Wait until the application is killed.
    thread.join();

    rt_printf("Exit cleanly \n");

    return 0;
}
