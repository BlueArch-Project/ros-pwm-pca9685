#include <pwm_pca9685/pca9685_activity.h>
#include <csignal>

pwm_pca9685::PCA9685Activity* activity = NULL;

void signalHandler(int signum) {
    if (activity) {
        activity->stop();  // カスタムクリーンアップ処理
        delete activity;
        activity = NULL;
    }
    ros::shutdown();  // ROSのクリーンアップ処理
    exit(signum);  // プロセスを終了
}

int main(int argc, char *argv[]) {
    ros::init(argc, argv, "pca9685_node", ros::init_options::NoSigintHandler);
    ros::NodeHandle* nh = NULL;
    ros::NodeHandle* nh_priv = NULL;

    // シグナルハンドラを設定
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    nh = new ros::NodeHandle();
    if(!nh) {
        ROS_FATAL("Failed to initialize NodeHandle");
        ros::shutdown();
        return -1;
    }

    nh_priv = new ros::NodeHandle("~");
    if(!nh_priv) {
        ROS_FATAL("Failed to initialize private NodeHandle");
        delete nh;
        ros::shutdown();
        return -2;
    }

    activity = new pwm_pca9685::PCA9685Activity(*nh, *nh_priv);
    if(!activity) {
        ROS_FATAL("Failed to initialize driver");
        delete nh_priv;
        delete nh;
        ros::shutdown();
        return -3;
    }

    if(!activity->start()) {
        ROS_ERROR("Failed to start activity");
        delete activity;
        delete nh_priv;
        delete nh;
        ros::shutdown();
        return -4;
    }

    ros::Rate rate(100);
    while(ros::ok()) {
        rate.sleep();
        activity->spinOnce();
    }

    // ループ終了後のクリーンアップ処理
    activity->stop();
    delete activity;
    delete nh_priv;
    delete nh;

    return 0;
}
