#this is final version that I have done
import socket
import time
import struct
import os
import threading
from datetime import datetime
import cv2

# global variable to save send timestamp
send_timestamps = []

# Function to handle frame sending
def send_frames(client_socket, encoded_frame, i):
    global send_timestamps
    
    # Get the encoded frame
    data = encoded_frame

    # Get current timestamp and save it
    timestamp = float(time.time())
    send_timestamps.append(timestamp)

    # Pack the frame data and header into a message
    timestamp_packed = struct.pack('d', timestamp)
    frame_size_packed = struct.pack('L', len(data))
    message = timestamp_packed + frame_size_packed  + data

    # Send the message to the server
    client_socket.sendall(message)

    print('Sent frame', i+1, 'of size', len(data), 'to the server')

 # Now receive the response from server
    receive_frame(client_socket, timestamp, len(data))

# Function to handle frame receiving
def receive_frame(client_socket, sent_timestamp, frame_size):
    # Receive the frame data back from the server
    received_frame_data = b''
    while len(received_frame_data) < frame_size:
        chunk = client_socket.recv(frame_size - len(received_frame_data))
        if not chunk:
            return
        received_frame_data += chunk

    # Calculate E2E delay
    e2e_delay = float(time.time()) - sent_timestamp
    print('E2E delay:', e2e_delay)
    rec_frame_len = len(received_frame_data)
    with open(filename, 'a') as f:
        f.write('Received frame with sent_timestamp ,{}, E2E delay ,{}, and size ,{},\n'.format(sent_timestamp,e2e_delay, rec_frame_len))


        

if not os.path.exists('./logging'):
    os.makedirs('./logging')
# Get the current date
now = datetime.now()

# Format the date as a string in the format 'YYYYMMDD'
date_string = now.strftime('%y%m%d')

# Use the date string in the file name
filename = './logging/video_analytics_client_log{}.txt'.format(date_string)

with open(filename, 'a') as f:
    f.write('start--------------------------------------------\n')

# Extract frames from the video and store them in a list
video_path = './test_video_data.3gpp'  # update with your video path
video = cv2.VideoCapture(video_path)
frames = []
while True:
    success, frame = video.read()
    if not success:
        break
    frames.append(frame)

# Convert frames to JPEG-encoded byte-like objects
encoded_frames = []
for frame in frames:
    success, encoded_image = cv2.imencode('.jpg', frame)
    if not success:
        raise ValueError('Could not encode image')
    encoded_frames.append(encoded_image.tobytes())

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Enable MPTCP
client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
client_socket.setsockopt(socket.IPPROTO_TCP, 42, 1) #42 is MPTCP_ENABLED option

# Connect to the server
server_address = ('54.180.119.186', 8888)
print('Connecting to %s:%s...' % server_address)
client_socket.connect(server_address)

frame_rate = 30

# Start threads for sending and receiving
threads = []
for i, encoded_frame in enumerate(encoded_frames):
    send_thread = threading.Thread(target=send_frames, args=(client_socket, encoded_frame, i))
    threads.append(send_thread)
    send_thread.start()

    # Wait for the next frame to be transmitted
    time.sleep(1/frame_rate)

# Wait for all threads to finish
for thread in threads:
    thread.join()



# Close the connection
client_socket.close()
