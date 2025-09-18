FROM registry.fedoraproject.org/fedora:39 AS builder

RUN dnf install -y git libcouchbase-devel php-devel php-pear gcc g++ make autoconf automake && dnf clean all

WORKDIR /build

RUN git clone --branch connections-count-reproducer https://github.com/avsej/php-couchbase .

RUN pecl package && pecl install ./couchbase-*.tgz

FROM registry.fedoraproject.org/fedora:39

RUN dnf install -y php php-fpm libcouchbase nginx less && dnf clean all

RUN \
    sed -i -E 's/(listen\s+)(80;)/\18083;/g' /etc/nginx/nginx.conf && \
    sed -i -E 's/[;\s]*catch_workers_output.*/catch_workers_output = yes/g' /etc/php-fpm.d/www.conf && \
    sed -i -E 's/[;\s]*pm\s*=.*/pm = ondemand/g' /etc/php-fpm.d/www.conf && \
    sed -i -E 's/[;\s]*pm\.max_children.*/pm.max_children = 10/g' /etc/php-fpm.d/www.conf && \
    sed -i -E 's/[;\s]*pm\.process_idle_timeout.*/pm.process_idle_timeout = 300s/g' /etc/php-fpm.d/www.conf

COPY --from=builder /usr/lib64/php/modules/couchbase.so /usr/lib64/php/modules/
COPY --from=builder /build/bin/docker/90-couchbase.ini /etc/php.d/
COPY --from=builder /build/bin/docker/*.php /usr/share/nginx/html/
COPY --from=builder --chmod=0755 /build/bin/docker/start.sh /usr/local/bin/start.sh

EXPOSE 8083
CMD ["/usr/local/bin/start.sh"]

