import socket
import time
import struct
import os
import threading
# global variable to save send timestamp
send_timestamps = []

# Function to handle frame sending
def send_frames(client_socket, frame_sizes):
    global send_timestamps
    
    # Define the frame rate (in frames per second)
    frame_rate = 30

    # Define the duration of the video transmission (in seconds)
    duration = 10

    # Transmit the video frames
    for i in range(frame_rate * duration):
        # Read the size of the next video frame from the txt file
        frame_size = int(frame_sizes[i % len(frame_sizes)])

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

        # Wait for the next frame to be transmitted
        time.sleep(1/frame_rate)

# Function to handle frame receiving
def receive_frames(client_socket, frame_sizes):
    global send_timestamps
    
    # Keep receiving the frames
    while True:
        # Read the size of the next video frame from the txt file
        frame_size = int(frame_sizes[0])

        # Receive the frame data back from the server
        received_frame_data = b''
        while len(received_frame_data) < frame_size:
            chunk = client_socket.recv(frame_size - len(received_frame_data))
            if not chunk:
                return
            received_frame_data += chunk

        # Calculate E2E delay
        if send_timestamps:
            send_timestamp = send_timestamps.pop(0)
            e2e_delay = time.time() - send_timestamp
            print('E2E delay:', e2e_delay)

        print('Received frame with size:', len(received_frame_data))

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
    #frame_sizes = ['81920']
    frame_sizes = ['327680']

while True:
    # Start threads for sending and receiving
    send_thread = threading.Thread(target=send_frames, args=(client_socket, frame_sizes))
    receive_thread = threading.Thread(target=receive_frames, args=(client_socket, frame_sizes))
    send_thread.start()
    receive_thread.start()

    # Wait for both threads to finish
    send_thread.join()
    receive_thread.join()

# Close the connection
client_socket.close()
