#include "../includes/shortcuts.h"
#include <getopt.h>
#include "../includes/uvc_extension_unit_ctrl.h"
#include "../includes/extend_cam_ctrl.h" // Ensure this includes the declaration for video_capture_save_raw
#include "../includes/ui_control.h"
#include "../includes/cam_property.h"
#include "../includes/v4l2_devices.h"
#include "../includes/json_parser.h"

#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

int v4l2_dev; /** global variable, file descriptor for camera device */
int fw_rev;   /** global variable, firmware revision for the camera */
struct v4l2_fract time_per_frame = {1, 15};

static struct option opts[] = {
    {"nbufs", 1, 0, 'n'},
    {"size", 1, 0, 's'},
    {"time-per-frame", 1, 0, 't'},
    {0, 0, 0, 0}
};

/** Function to set terminal to raw mode for reading single characters */
void set_raw_mode() {
    struct termios new_settings;
    tcgetattr(STDIN_FILENO, &new_settings);
    new_settings.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
}

/** Function to reset terminal to default mode */
void reset_terminal_mode() {
    struct termios new_settings;
    tcgetattr(STDIN_FILENO, &new_settings);
    new_settings.c_lflag |= (ICANON | ECHO); // Enable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
}

/** main function */
int main(int argc, char **argv) {
    struct device dev;
    char dev_name[64] = "/dev/video0";

    int do_set_format = 0;
    int do_set_time_per_frame = 0;
    char *endptr;
    dev.nbufs = V4L_BUFFERS_DEFAULT;
    int c;
    int sys_ret;

    char *ret_dev_name = enum_v4l2_device(dev_name);
    v4l2_dev = open_v4l2_device(ret_dev_name, &dev);

    if (v4l2_dev < 0) {
        printf("open camera %s failed, err code: %d\n\r", dev_name, v4l2_dev);
        return -1;
    }

    printf("********************List Available Resolutions***************\n");
    sys_ret = system("v4l2-ctl --list-formats-ext | grep Size | awk '{print $1 $3}'| sed 's/Size/Resolution/g'");
    if (sys_ret < 0) {
        printf("failed to list camera %s resolution\n\r", dev_name);
        return -1;
    }

    printf("********************Camera Tool Usages***********************\n");
    while ((c = getopt_long(argc, argv, "n:s:t:", opts, NULL)) != -1) {
        switch (c) {
            case 'n':
                dev.nbufs = atoi(optarg);
                if (dev.nbufs > V4L_BUFFERS_MAX)
                    dev.nbufs = V4L_BUFFERS_MAX;
                printf("device nbuf %d\n", dev.nbufs);
                break;
            case 's':
                do_set_format = 1;
                dev.width = strtol(optarg, &endptr, 10);
                if (*endptr != 'x' || endptr == optarg) {
                    printf("Invalid size '%s'\n", optarg);
                    return 1;
                }
                dev.height = strtol(endptr + 1, &endptr, 10);
                if (*endptr != 0) {
                    printf("Invalid size '%s'\n", optarg);
                    return 1;
                }
                break;
            case 't':
                do_set_time_per_frame = 1;
                time_per_frame.denominator = atoi(optarg);
                break;
            default:
                printf("Invalid option -%c\n", c);
                printf("Run %s -h for help.\n", argv[0]);
                return -1;
        }
    }

    if (optind >= argc) {
        usage(argv[0]);
    }

    /** Set the video format. */
    if (do_set_format) {
        video_set_format(&dev, dev.width, dev.height, V4L2_PIX_FMT_YUYV);
    }

    /** Set the frame rate. */
    if (do_set_time_per_frame) {
        set_frame_rate(v4l2_dev, time_per_frame.denominator);
    }

    printf("********************Device Information************************\n");
    fw_rev = read_cam_uuid_hwfw_rev(v4l2_dev);
    check_dev_cap(&dev);
    video_get_format(&dev); /** list the current resolution etc */
    get_frame_rate(v4l2_dev); /** list the current frame rate */

    printf("********************Allocate Buffer for Capturing************\n");
    video_alloc_buffers(&dev, dev.nbufs);

    printf("********************Control Logs*****************************\n");
    start_camera(&dev);
   
    // Fork process
    pid_t pid = fork();
    if (pid == 0) {
        /** Child process */
        set_raw_mode();
        printf("Child process: Press Enter to capture raw video...\n");
       
        while (true) {
            if (getchar() == '\n') { // Wait for Enter key press
                video_capture_save_raw(v4l2_dev); // Call the function
                printf("Child process: Captured raw video.\n");
            }
        }
       
        reset_terminal_mode(); // Reset terminal settings in child (optional)
        _exit(0); // Exit child process
    } else if (pid > 0) {
        /** Parent process */
        streaming_loop(&dev); // Continue streaming
    } else {
        fprintf(stderr, "ERROR: fork() failed\n");
    }

    /** Cleanup and deallocate resources */
    stop_Camera(&dev);
    video_free_buffers(&dev);
    close(v4l2_dev);

    sys_ret = system("killall -9 leopard_cam");
    if (sys_ret < 0) {
        printf("fail to exit the leopard camera tool\r\n");
        return -1;
    }
   
    return 0;
}

