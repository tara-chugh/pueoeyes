"""
Commands to run: 

1. make leopard cam (if the C++ source code was modified)
2. sudo python3 take_pic.py (don't need leopard cam streaming in another terminal) - don't always need to run in sudo mode

May need to do sudo killall -9 leopard_cam if you get error, since I haven't figured out cleanup yet.

'VIDIOC_DQBUF: Invalid argument' error indicates that camera needs to be re-initialized by disconnecting power and USB,
then re-connecting power then USB.

If it correctly takes an image, it should print these two lines:
    V4L2_CORE (with updates): saved data to captures_LI-S5K2G1_0.raw
    Saved PNG image to to <filename>

"""

import os
import signal
import subprocess
import time

# Run the C++ executable as a child process
process = subprocess.Popen(
    ["sudo", "-E", "./leopard_cam", "capture"], preexec_fn=os.setsid
)

try:
    # Sleep so the camera can initialize and get ready to capture the image.
    time.sleep(5)

    # Send SIGUSR1 to the child process (signals camera to take a picture)
    os.kill(process.pid, signal.SIGUSR1)
    print("Signal sent to capture an image. Camera program should print that it has received the signal.")

    # Sleep for 10 seconds before exiting to ensure that the camera has time to take the picture and save it.
    # 10 seconds is a conservative estimate for how long the picture-taking should take, it normally
    # takes <5 seconds.
    time.sleep(30)

    os.kill(process.pid, signal.SIGUSR2)  # Clean up leopard_cam process and release buffer 
    time.sleep(2)
    print("Exiting the Python script.")
    os.kill(process.pid, signal.SIGTERM) # Terminate the C++ process

except Exception as E:
    print(f"Camera failed to take a picture due to {E}")
