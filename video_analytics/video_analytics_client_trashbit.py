import socket
import time
import struct
import os
import threading
from datetime import datetime


# global variable to save send timestamp
send_timestamps = []

# Function to handle frame sending
def send_frames(client_socket,frame_sizes):
    global send_timestamps
    
    # Transmit the video frames
    
    frame_size = int(frame_sizes)
    
    # Create a video frame of the specified size
    data = b'0' * frame_size
    for i in range(frame_rate * duration):
        # Get current timestamp and save it
        timestamp = float(time.time())
        send_timestamps.append(timestamp)

        # Pack the frame data and header into a message
        timestamp_packed = struct.pack('t', timestamp)
        frame_size_packed = struct.pack('f', frame_size)
        index_packed = struct.pack('i', i)
        message = timestamp_packed + frame_size_packed + index_packed  + data

        # Send the message to the client
        client_socket.sendall(message)

        print('Sent frame', i+1, 'of size', frame_size, 'to the client')
        
    #receive_frames(client_socket,i,timestamp,len(data))
    # Wait for the next frame to be transmitted
    

# Function to handle frame receiving
def receive_frames(client_socket, frame_size):
    # Receive the frame data back from the server
    while True:
        received_frame_data = b''
        while len(received_frame_data) < frame_size:
            chunk = client_socket.recv(frame_size - len(received_frame_data))
            if not chunk:
                break
            received_frame_data += chunk
    
    # Receive the video frames from the server
    while True:
        # Receive the message header from the server
        header_data = b''
        while len(header_data) < 16:
            chunk = client_socket.recv(16 - len(header_data))
            if not chunk:
                break
            header_data += chunk
        if len(header_data) < 16:
            break
    
        print('Header', len(header_data))
        # Unpack the timestamp and frame size fields from the message header
        sent_timestamp, frame_size, idx = struct.unpack('tfi', header_data)
    
        # Receive the frame data from the server
        frame_data = b''
        while len(frame_data) < frame_size:
            chunk = client_socket.recv(frame_size - len(frame_data))
            if not chunk:
                break
            frame_data += chunk
        if len(frame_data) < frame_size:
            break
        
        # Calculate E2E delay
        received_timestamp = float(time.time()) + 0.06 # 60ms for video analytics processing time on server side
        received_send_delay = received_timestamp - sent_timestamp 
        print('received_send_delay:', received_send_delay)
        rec_frame_len = len(received_frame_data)
        with open(filename, 'a') as f:
            f.write('packet_index ,{}, sent_timestamp ,{}, received_timestamp,{},received-send delay ,{}, and size ,{},\n'.format(idx,sent_timestamp,received_timestamp,received_send_delay, rec_frame_len))

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
    
    frame_sizes = ['81920']
    #frame_sizes = ['327680']
    print('Error: No frame_sizes.txt file found. Using default frame size of {}KB'.format(frame_sizes[0]/1024))

threads = []
# Define the frame rate (in frames per second)
frame_rate = 30

# Define the duration of the video transmission (in seconds)
duration = 10

# Start threads for sending and receiving

send_thread = threading.Thread(target=send_frames(client_socket,frame_sizes[0]))
send_thread.start()
receive_thread = threading.Thread(target=receive_frames(client_socket, sent_timestamp, frame_sizes[0])
receive_thread.start()                                  
send_thread.join()
receive_thread.join()                                  
                                


# Close the connection
client_socket.close()
