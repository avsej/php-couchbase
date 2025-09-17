#!/usr/bin/bash

mkdir -p /run/php-fpm
php-fpm
nginx
while true
do
    ls -l /var/log/php-fpm
    tail -f /var/log/php-fpm/www-error.log || sleep 1
done
