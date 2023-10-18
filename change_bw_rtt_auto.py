#!/usr/bin/env python3
import subprocess
import argparse
import pandas as pd
import time

def change_network_conditions_from_csv(file):
    df = pd.read_csv(file)  # CSV 파일 읽기

    while True:
        for index, row in df.iterrows():
            # WiFi0에 대한 bandwidth와 latency 값을 추출 및 설정
            bandwidth_WiFi0 = row['bandwidth_WiFi0']
            latency_WiFi0 = row['delay_WiFi0']
            cmd_WiFi0 = f'sudo tc qdisc replace dev WiFi0 root netem rate {bandwidth_WiFi0}mbit delay {latency_WiFi0}ms limit 1800'
            print(cmd_WiFi0)
            subprocess.run(cmd_WiFi0, shell=True, check=True)

            # WiFi1에 대한 bandwidth와 latency 값을 추출 및 설정
            bandwidth_WiFi1 = row['bandwidth_WiFi1']
            latency_WiFi1 = row['delay_WiFi1']
            cmd_WiFi1 = f'sudo tc qdisc replace dev WiFi1 root netem rate {bandwidth_WiFi1}mbit delay {latency_WiFi1}ms limit 1800'
            print(cmd_WiFi1)
            subprocess.run(cmd_WiFi1, shell=True, check=True)

            # 다음 반복 전에 100ms 대기
            time.sleep(0.1)

def main():
    parser = argparse.ArgumentParser(description='Change network conditions for outbound traffic on both WiFi0 and WiFi1 interfaces.')
    parser.add_argument('--file', type=str, help='The CSV file to read network conditions from.')
    args = parser.parse_args()

    change_network_conditions_from_csv(args.file)

if __name__ == "__main__":
    main()
