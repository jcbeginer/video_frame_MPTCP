import socket
import time
import struct
import os
import threading
from datetime import datetime
import pytz



# Function to handle frame sending
def send_frames(client_socket,frame_sizes):
    #global send_timestamps
    
    # Transmit the video frames
    
    frame_size = int(frame_sizes)
    
    # Create a video frame of the specified size
    data = b'0' * frame_size
    timestamp = float(time.time())
    for i in range(frame_rate * duration):
        # Get current timestamp and save it
        tmp = float(time.time())
        with open(filename2, 'a') as f:
            f.write('packet_index is {} and delayed_time is {}\n'.format(i+1,tmp-timestamp))
        timestamp = float(time.time())
        #send_timestamps.append(timestamp)

        # Pack the frame data and header into a message
        timestamp_packed = struct.pack('d', timestamp)
        frame_size_packed = struct.pack('L', frame_size)
        index_packed = struct.pack('i', i+1)
        message = timestamp_packed + frame_size_packed + index_packed  + data

        # Send the message to the client
        client_socket.sendall(message)
        delayed_time = float(time.time()) - timestamp
        print('Sent frame', i+1, 'of size', frame_size, 'to the client')
        if delayed_time >1/frame_rate: continue
        else: wait_time = (1/frame_rate)-delayed_time
            time.sleep(wait_time)
        
    #receive_frames(client_socket,i,timestamp,len(data))
    # Wait for the next frame to be transmitted
    

# Function to handle frame receiving
def receive_frames(client_socket, frame_size):
    print("receiver start")
    kst = pytz.timezone('Asia/Seoul')
    
    frame_size = int(frame_size)
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

      #print('Header', len(header_data))
      sent_timestamp, received_frame_size, idx = struct.unpack('dLi', header_data)
        
      frame_data = b''
      while len(frame_data) < frame_size:
          chunk = client_socket.recv(frame_size - len(frame_data))
          if not chunk:
              break
          frame_data += chunk
      if len(frame_data) < frame_size:
          break
        
        # Calculate E2E delay
        # 60ms for video analytics processing time on server side
      received_timestamp = float(time.time()) +  0.02063 
      received_send_delay = received_timestamp - sent_timestamp 
      print('packet_idx {}, received_send_delay {}'.format(idx, received_send_delay))
      rec_frame_len = len(frame_data)
      timestamp_str = datetime.fromtimestamp(sent_timestamp, tz=kst).strftime('%y-%m-%d %H:%M:%S')
      with open(filename, 'a') as f:
          f.write('packet_index ,{}, sent_timestamp ,{},received-send delay ,{}, and size ,{},\n'.format(idx,timestamp_str,received_send_delay, send_frame_size))

if not os.path.exists('./logging'):
    os.makedirs('./logging')
# Get the current date
now = datetime.now()

# Format the date as a string in the format 'YYYYMMDD'
date_string = now.strftime('%y%m%d')

# Use the date string in the file name
filename = './logging/video_analytics_client_log{}_minRTT_40KB.txt'.format(date_string)
filename2 = './logging/delayed_time_log{}.txt'.format(date_string)

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
    
    frame_sizes=[15360,25600,30720,40960,71680,102400,133120,153600] # 15KB (index24), 25KB(index25), 30KB(index26), 40KB(index27), 70KB(index28), 100KB(index29), 130KB(index30), 150KB(index31)
    #frame_sizes = ['81920']
    #frame_sizes = ['327680']
    print('Error: No frame_sizes.txt file found. Using default frame size of {}KB'.format(int(int(frame_sizes[0])/1024)))

#define frame_size
send_frame_size = frame_sizes[3]
threads = []
# Define the frame rate (in frames per second)
frame_rate = 30

# Define the duration of the video transmission (in seconds)
duration = 100

# Start threads for sending and receiving

send_thread = threading.Thread(target=send_frames,args=(client_socket,send_frame_size))
receive_thread = threading.Thread(target=receive_frames,args=(client_socket, 8192))
receive_thread.start()                                  
send_thread.start()
send_thread.join()
receive_thread.join()                                  
                                


# Close the connection
client_socket.close()
