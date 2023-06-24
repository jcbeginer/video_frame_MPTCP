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
        timestamp_packed = struct.pack('d', timestamp)
        frame_size_packed = struct.pack('L', frame_size)
        index_packed = struct.pack('i', i)
        message = timestamp_packed + frame_size_packed + index_packed  + data

        # Send the message to the client
        client_socket.sendall(message)
       
        print('Sent frame', i+1, 'of size', frame_size, 'to the client')
        time.sleep(1/frame_rate)
        
    #receive_frames(client_socket,i,timestamp,len(data))
    # Wait for the next frame to be transmitted
    

# Function to handle frame receiving
def receive_frames(client_socket, frame_sizes):
    frame_size = int(frame_sizes)
    # Receive the frame data back from the server
    while True:
      header_data = b''
      while len(header_data) < 20:
          chunk = client_socket.recv(20 - len(header_data))
          if not chunk:
              break
          header_data += chunk
      if len(header_data) < 20:
          break

      print('Header', len(header_data))
      timestamp, frame_size, idx = struct.unpack('dLi', header_data)
        
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
        print('packet_idx {}, received_send_delay {}'.format(idx, received_send_delay))
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
    
    frame_sizes=['800']
    #frame_sizes = ['81920']
    #frame_sizes = ['327680']
    print('Error: No frame_sizes.txt file found. Using default frame size of {}KB'.format(int(int(frame_sizes[0])/1024)))

threads = []
# Define the frame rate (in frames per second)
frame_rate = 30

# Define the duration of the video transmission (in seconds)
duration = 10

# Start threads for sending and receiving

send_thread = threading.Thread(target=send_frames,args=(client_socket,frame_sizes[0]))
receive_thread = threading.Thread(target=receive_frames,args=(client_socket, frame_sizes[0]))
receive_thread.start()                                  
send_thread.start()
send_thread.join()
receive_thread.join()                                  
                                


# Close the connection
client_socket.close()
