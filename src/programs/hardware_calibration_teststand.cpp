/**
 * @file
 * @license BSD 3-clause
 * @copyright Copyright (c) 2020, New York University and Max Planck
 * Gesellschaft
 *
 * @brief This program is used to perform the hardware calibration of the robot.
 * Procedure:
 *  - You must start the robot in a position that is close to the "zero" pose.
 *  - Then start the demo: roscd && sudo ./lib/teststand/hardware_calibration
 * <network_id> The "network_id" is the left column of the output of `ifconfig`
 * in bash.
 *  - The robot is going to find the closest index in a specific direction and
 *      go idle (0 torques sent).
 *  - In the terminal you have displayed the distance between the zero and the
 *      found index. Therefore once you placed the joints to the proper zero
 *      configuration you can just kill the application with ctrl+c and copy the
 *      date to you favorite configuration file.
 */

#include "teststand/utils.hpp"
#include "teststand/teststand.hpp"

using namespace teststand;

static THREAD_FUNCTION_RETURN_TYPE control_loop(void* robot_void_ptr)
{
    Teststand& robot = *(static_cast<Teststand*>(robot_void_ptr));

    Eigen::Vector2d joint_index_to_zero = Eigen::Vector2d::Zero();
    Eigen::Vector2d dummy_command = Eigen::Vector2d::Zero();

    robot.wait_until_ready();

    robot.request_calibration(joint_index_to_zero);

    real_time_tools::Spinner spinner;
    spinner.set_period(0.001);
    rt_printf("Running calibration...\n");
    while (!CTRL_C_DETECTED && robot.is_calibrating())
    {
        robot.acquire_sensors();
        robot.send_target_joint_torque(dummy_command);
        spinner.spin();
    }

    // re-send 0 torque command
    robot.send_target_joint_torque(dummy_command);

    rt_printf("Running calibration... Done.\n");
    int count = 0;
    spinner.set_period(0.001);
    while (!CTRL_C_DETECTED)
    {
        robot.acquire_sensors();
        if (count % 100 == 0){
            print_vector("Joint Positions", -robot.get_joint_positions());
        }
        // re-send 0 torque command
        robot.send_target_joint_torque(dummy_command);
        spinner.spin();
        ++count;
    }

    return THREAD_FUNCTION_RETURN_VALUE;
}  // end control_loop

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        throw std::runtime_error(
            "Wrong number of argument: `sudo ./hardware_calibration "
            "network_id`.");
    }

    enable_ctrl_c();

    real_time_tools::RealTimeThread thread;

    Teststand robot;
    robot.initialize(argv[1]);

    rt_printf("Controller is set up.\n");
    // rt_printf("Press enter to launch the calibration.\n");
    // char str[256];
    // std::cin.get(str, 256);  // get c-string

    thread.create_realtime_thread(&control_loop, &robot);

    // Wait until the application is killed.
    thread.join();

    rt_printf("Exit cleanly.\n");

    return 0;
}