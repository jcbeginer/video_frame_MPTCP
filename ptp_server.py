import socket
import time

SERVER_IP = '172.31.41.24'  # Replace with the server's IP address
PORT = 9090

def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.bind((SERVER_IP, PORT))
    
    while True:
        request, addr = server_socket.recvfrom(1024)
        if request.decode() == 'SYNC_REQUEST':
            timestamp = time.time()
            server_socket.sendto(str(timestamp).encode(), addr)

if __name__ == "__main__": 
    main()
