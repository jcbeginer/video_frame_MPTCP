# video_frame_MPTCP

for UL scenario using MPTCP


ref:https://github.com/lgs96/simple-socket-python

# ptpd setup

I used offical code
ref: https://github.com/ptpd/ptpd

and config file is for time sync between client and server

(client is master and server is slave)

before start MPTCP connection, by using this we can make time sync

$sudo ./ptpd2 -c ./ptpd_server.conf
$sudo ./ptpd2 -c ./ptpd_client.conf

you don't need to stop ptpd2 on server. but you need to stop ptpd2 on client because it send ping continuously (it can be make experiment result strangely)

# ptp_client.py & ptp_server.py

this code is provided by my coworker gslee.
this codes are simple version of ptpd
through this code we can also make time sync.
(but we can't guarantee 100% performance comparing to ptpd_official)

