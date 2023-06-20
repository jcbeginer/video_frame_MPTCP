import socket
import time
import struct
import os
import threading

# Handle client connection
def handle_client(client_socket):
    print('Accepted connection from:', client_address)

    if not os.path.exists('./logging'):
        os.makedirs('./logging')

    with open('./logging/video_analytics_log_file.txt', 'a') as f:
        f.write('start--------------------------------------------\n')

    while True:
        header_data = b''
        while len(header_data) < 16:
            chunk = client_socket.recv(16 - len(header_data))
            if not chunk:
                break
            header_data += chunk
        if len(header_data) < 16:
            break

        print('Header', len(header_data))
        timestamp, frame_size = struct.unpack('dL', header_data)

        frame_data = b''
        while len(frame_data) < frame_size:
            chunk = client_socket.recv(frame_size - len(frame_data))
            if not chunk:
                break
            frame_data += chunk
        if len(frame_data) < frame_size:
            break

        e2e_delay = float(time.time())-timestamp

        print('Received frame with timestamp:', timestamp,'E2E delay:',e2e_delay, 'and size:', frame_size)

        time.sleep(0.06)
        client_socket.sendall(frame_data)

        with open('./logging/video_analytics_log_file.txt', 'a') as f:
            f.write('Received frame with timestamp ,{}, E2E delay ,{}, and size ,{},\n'.format(timestamp,e2e_delay, frame_size))


# Close the server socket
# This will never be reached in the current script, as the while True loop does not terminate
# It would need to be called in a shutdown routine

def terminate(client_socket, server_socket):
    client_socket.close()
    server_socket.close()
    with open('./logging/video_analytics_log_file.txt', 'a') as f:
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
while True:
    # Accept incoming connections
    client_socket, client_address = server_socket.accept()

    # Start a new thread to handle this client
    client_thread = threading.Thread(target=handle_client, args=(client_socket,))
    client_thread.start()

    #termination part
    '''
    if float(time.time()) - timer > 10:
        terminate()
    else :
        timer = float(time.time())
    '''
