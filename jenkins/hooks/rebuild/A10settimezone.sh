#!/bin/bash

if cat /etc/debian_version | grep -q '9'; then
    echo "Setting up Stretch"
    ln -fs /usr/share/zoneinfo/Europe/Prague /etc/localtime
    dpkg-reconfigure --frontend noninteractive tzdata
else
    echo "Setting up other system"
    echo "Europe/Prague" | tee /etc/timezone
    dpkg-reconfigure --frontend noninteractive tzdata
fi
