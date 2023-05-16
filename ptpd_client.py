import socket
import time

SERVER_IP = '54.180.119.186'  # Replace with the server's IP address
PORT = 9090

def synchronize_time():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    send_time = time.time()
    client_socket.sendto('SYNC_REQUEST'.encode(), (SERVER_IP, PORT))

    server_timestamp = client_socket.recv(1024).decode()
    receive_time = time.time()

    round_trip_time = receive_time - send_time
    adjusted_server_time = float(server_timestamp) + (round_trip_time / 2)

    return adjusted_server_time

def main():
    print('Start client')
    adjusted_server_time = synchronize_time()
    print(f'Adjusted server time: {adjusted_server_time}')

if __name__ == "__main__":
    main()
