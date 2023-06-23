import socket
import time
import struct
import os
import threading
from datetime import datetime
import queue

# global variable to save send timestamp
send_timestamps = []

# Create a Queue object
q = queue.Queue()

for i in range(frame_rate * duration):
    q.put((client_socket, i, frame_sizes[0]))

# Function to handle frame sending
def send_frames(client_socket, i,frame_sizes):
    global send_timestamps
    
    # Transmit the video frames
    
    frame_size = int(frame_sizes)

    # Create a video frame of the specified size
    data = b'0' * frame_size

    # Get current timestamp and save it
    timestamp = float(time.time())
    send_timestamps.append(timestamp)

    # Pack the frame data and header into a message
    timestamp_packed = struct.pack('d', timestamp)
    frame_size_packed = struct.pack('L', frame_size)
    message = timestamp_packed + frame_size_packed  + data

    # Send the message to the client
    client_socket.sendall(message)

    print('Sent frame', i+1, 'of size', frame_size, 'to the client')
        
    receive_frames(client_socket,i,timestamp,len(data))
    # Wait for the next frame to be transmitted
    

# Function to handle frame receiving
def receive_frames(client_socket, i,sent_timestamp, frame_size):
    # Receive the frame data back from the server
    received_frame_data = b''
    while len(received_frame_data) < frame_size:
        chunk = client_socket.recv(frame_size - len(received_frame_data))
        if not chunk:
            return
        received_frame_data += chunk

    # Calculate E2E delay
    received_timestamp = float(time.time())
    received_send_delay = received_timestamp - sent_timestamp
    print('received_send_delay:', received_send_delay)
    rec_frame_len = len(received_frame_data)
    with open(filename, 'a') as f:
        f.write('number ,{}, sent_timestamp ,{}, received_timestamp,{},received-send delay ,{}, and size ,{},\n'.format(i,sent_timestamp,received_timestamp,received_send_delay, rec_frame_len))

# Function to process frames
def process_frames():
    while not q.empty():
        args = q.get()
        send_frames(*args)
        time.sleep(1/frame_rate)

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

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Enable MPTCP
client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
client_socket.setsockopt(socket.IPPROTO_TCP, 42, 1) #42 is MPTCP_ENABLED option

# Connect to the server
server_address = ('54.180.119.186', 8888)
print('Connecting to %s:%s...' % server_address)
client_socket.connect(server_address)

# Read the frame sizes from the txt file
try:
    with open('frame_sizes.txt', 'r') as f:
        frame_sizes = f.read().splitlines()
except FileNotFoundError:
    print('Error: No frame_sizes.txt file found. Using default frame size of 80KB')
    frame_sizes = ['81920']
    #frame_sizes = ['327680']

threads = []
# Define the frame rate (in frames per second)
frame_rate = 30

# Define the duration of the video transmission (in seconds)
duration = 10

# Start threads for sending and receiving
while not q.empty():
    send_thread = threading.Thread(target=process_frames)
    threads.append(send_thread)
    send_thread.start()
    time.sleep(1/frame_rate)

# Wait for threads to finish
for thread in threads:
    thread.join()


# Close the connection
client_socket.close()
