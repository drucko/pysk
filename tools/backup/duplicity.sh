#/bin/bash
# -*- coding: utf-8 -*-

# Copyright 2006 - 2012 Philipp Wollermann
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e
set -u

FTP_HOST="88.198.42.117"
FTP_USER="XXXXX"
FTP_PASSWORD="XXXXXXXX"
FTP_URL="ftp://$FTP_USER@$FTP_HOST/duplicity/`hostname`/"
PASSPHRASE="XXXXXXXXXXXXXXX"

set -x

lftp -c "open $FTP_HOST && login $FTP_USER $FTP_PASSWORD && cd duplicity && mkdir `hostname`" &> /dev/null || /bin/true

FTP_PASSWORD=$FTP_PASSWORD PASSPHRASE=$PASSPHRASE duplicity --full-if-older-than 14D --encrypt-key "FA3B07B8" --sign-key "FA3B07B8" -v8 \
    --gpg-options='--compress-algo=bzip2 --bzip2-compress-level=9' --asynchronous-upload \
    --exclude /bin --exclude /boot --exclude /dev --exclude /lib --exclude /lib64 \
    --exclude /lost+found --exclude /media --exclude /mnt --exclude /opt \
    --exclude /proc --exclude /sbin --exclude /srv --exclude /sys --exclude /tmp \
    --exclude /usr --exclude /var/cache --exclude /var/lib/mysql --exclude /var/lib/postgres \
    --exclude /var/lib/postgres-8.3 --exclude /var/lock --exclude /var/tmp \
    --exclude "/var/log/*log.*" --exclude "/var/log/**/*log.*" --exclude /var/log/btmp \
    --exclude /var/lib/pacman --exclude /home/mirror --exclude /root/.cache/duplicity/ \
    --exclude /var/abs --exclude /swapfile \
    / ${FTP_URL}
FTP_PASSWORD=$FTP_PASSWORD PASSPHRASE=$PASSPHRASE duplicity remove-all-but-n-full 1 --force ${FTP_URL}
FTP_PASSWORD=$FTP_PASSWORD PASSPHRASE=$PASSPHRASE duplicity cleanup --extra-clean --force ${FTP_URL}

