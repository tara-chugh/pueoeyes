#!/bin/bash

# Define variables
REMOTE_USER="camerapi"
REMOTE_HOST="10.42.0.180"
REMOTE_DIR="/home/camerapi/linux_camera_tool-master"
REMOTE_PYTHON_SCRIPT="take_pic.py"
REMOTE_IMAGE_PATH="/home/camerapi/linux_camera_tool-master/captures_LI-S5K2G1_0.png"
REMOTE_OLD_IMAGES_DIR="/home/camerapi/linux_camera_tool-master/old_images"
LOCAL_SAVE_DIR="/home/lab-user/remote_camera_program"
SSH_PASSWORD="HarshContinent!"
SUDO_PASSWORD="HarshContinent!"  # Local machine sudo password for running commands

# Get current timestamp for renaming the image
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

echo "Logs being saved to $LOCAL_SAVE_DIR/logs_$TIMESTAMP.txt"

# Run Python script on the remote machine and wait until it finishes
sshpass -p "$SSH_PASSWORD" ssh "$REMOTE_USER@$REMOTE_HOST" \
    "echo '$SUDO_PASSWORD' | sudo -S bash -c 'cd $REMOTE_DIR && sudo python3 $REMOTE_PYTHON_SCRIPT'" \
    > "$LOCAL_SAVE_DIR/logs_$TIMESTAMP.txt"

# Check if the image file exists on the remote machine after the Python script has run
if sshpass -p "$SSH_PASSWORD" ssh "$REMOTE_USER@$REMOTE_HOST" "[ -f $REMOTE_IMAGE_PATH ]"; then
    # If the file exists, copy it to the local machine with a timestamped name
    sshpass -p "$SSH_PASSWORD" scp "$REMOTE_USER@$REMOTE_HOST:$REMOTE_IMAGE_PATH" \
        "$LOCAL_SAVE_DIR/image_$TIMESTAMP.png"
    echo "Image copied successfully to $LOCAL_SAVE_DIR/image_$TIMESTAMP.png"
    
    # Move the image to the 'old_images' subdirectory on the remote machine
    sshpass -p "$SSH_PASSWORD" ssh "$REMOTE_USER@$REMOTE_HOST" \
        "mkdir -p $REMOTE_OLD_IMAGES_DIR && mv $REMOTE_IMAGE_PATH $REMOTE_OLD_IMAGES_DIR/captures_LI-S5K2G1_$TIMESTAMP.png"
    echo "Image moved to $REMOTE_OLD_IMAGES_DIR on the remote machine."
else
    # Print an error message if the file does not exist
    echo "Error: Image file $REMOTE_IMAGE_PATH does not exist on the remote machine."
fi


