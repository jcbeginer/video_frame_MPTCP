import socket
import time
import struct
import os

# Create a socket object
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Enable MPTCP
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.setsockopt(socket.IPPROTO_TCP, 42, 1) # 42 is the MPTCP_ENABLED option

# Bind the socket object to a specific address and port
server_socket.bind(('172.31.41.24', 8888))

# Listen for incoming connections
server_socket.listen(1)

print('Waiting for incoming connections...')

# Accept incoming connections
client_socket, client_address = server_socket.accept()

print('Accepted connection from:', client_address)

# Ensure the logging directory exists
if not os.path.exists('./logging'):
    os.makedirs('./logging')

# start logging
with open('./logging/log_file.txt', 'a') as f:
        # Write the log message to the file
        f.write('start--------------------------------------------\n')

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
    timestamp, frame_size = struct.unpack('dL', header_data)
    
    # Receive the frame data from the server
    frame_data = b''
    while len(frame_data) < frame_size:
        chunk = client_socket.recv(frame_size - len(frame_data))
        if not chunk:
            break
        frame_data += chunk
    if len(frame_data) < frame_size:
        break
    e2e_delay = float(time.time())-timestamp
    # Print the timestamp and frame size
    print('Received frame with timestamp:', timestamp,'E2E delay:',e2e_delay, 'and size:', frame_size)
    
# Write log
    
    # Open the log file in append mode
    with open('./logging/log_file.txt', 'a') as f:
        # Write the log message to the file
        f.write('Received frame with timestamp ,{}, E2E delay ,{}, and size ,{},\n'.format(timestamp,e2e_delay, frame_size))

    
# Close the connection
client_socket.close()
server_socket.close()
with open('./logging/log_file.txt', 'a') as f:
    f.write('END ----------------------------------------------------------------------------')
