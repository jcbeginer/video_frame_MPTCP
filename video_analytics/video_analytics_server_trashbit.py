import socket
import time
import struct
import os
import threading
from datetime import datetime
import queue


# Handle client connection
def receive_frame():
    
      header_data = b''
      while len(header_data) < 16:
          chunk = client_socket.recv(16 - len(header_data))
          if not chunk:
              break
          header_data += chunk
      if len(header_data) < 16:
          break

      print('Header', len(header_data))
      timestamp, frame_size, idx = struct.unpack('tfi', header_data)
      received_timestamp = float(time.time())
      e2e_delay = received_timestamp-timestamp
        
      frame_data = b''
      while len(frame_data) < frame_size:
          chunk = client_socket.recv(frame_size - len(frame_data))
          if not chunk:
              break
          frame_data += chunk
      if len(frame_data) < frame_size:
          break
      # Pack the frame data and header into a message
      timestamp_packed = struct.pack('t', timestamp)
      frame_size_packed = struct.pack('f', frame_size)
      idx_packed = struct.pack('i', idx)
      message = timestamp_packed + frame_size_packed + idx_packed  + frame_data
                
      print('Received frame with timestamp:', timestamp,'E2E delay:',e2e_delay, 'and size:', frame_size)

      time.sleep(0.06)
      client_socket.sendall(message)

      with open(filename, 'a') as f:
          f.write('sender's timestamp ,{},received timestamp ,{}, E2E delay ,{}, and size ,{},\n'.format(timestamp,received_timestamp,e2e_delay, frame_size))


# Close the server socket
# This will never be reached in the current script, as the while True loop does not terminate
# It would need to be called in a shutdown routine

def terminate(client_socket, server_socket):
    client_socket.close()
    server_socket.close()
    with open(filename, 'a') as f:
        f.write('END ----------------------------------------------------------------------------\n')


# Create a socket object
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Enable MPTCP
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.setsockopt(socket.IPPROTO_TCP, 42, 1)

# Bind the socket object to a specific address and port
server_socket.bind(('172.31.41.24', 8888))

# Listen for incoming connections
server_socket.listen(1)

print('Waiting for incoming connections...')

# set timer for termination
# timer = float(time.time())

# Accept incoming connections
client_socket, client_address = server_socket.accept()

print('Accepted connection from:', client_address)

if not os.path.exists('./logging'):
    os.makedirs('./logging')
# Get the current date
now = datetime.now()

# Format the date as a string in the format 'YYYYMMDD'
date_string = now.strftime('%y%m%d')

# Use the date string in the file name
filename = './logging/video_analytics_server_log{}.txt'.format(date_string)

with open(filename, 'a') as f:
    f.write('start--------------------------------------------\n')

threads =[]
# Start a new thread to handle this client
while True:
    thread = threading.Thread(target=process_frames)
    threads.append(thread)
    thread.start()
  
for thread in threads:
  thread.join()

terminate(client_socket, server_socket)
