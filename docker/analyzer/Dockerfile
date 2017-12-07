FROM ubuntu
MAINTAINER Roberto Solar "roberto.solar@usach.cl"

WORKDIR /root

ADD . /root

RUN apt-get update
RUN apt-get install -y git cmake g++ libboost-dev libtool autoconf curl libarmadillo-dev libssl-dev libboost-system-dev libboost-thread-dev libcurl3-dev

#install libgdrift
RUN cd && git clone https://github.com/robertosolargallardo/libgdrift.git && cd libgdrift && mkdir build && cd build && cmake .. && make && make install

#install restbed
RUN cd && git clone --recursive https://github.com/corvusoft/restbed.git && cd restbed/dependency/openssl/ && ./config && make && make install && cd && mkdir restbed/build && cd restbed/build && cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make && make install && cd /usr/lib/ && ln -s /usr/library/librestbed.a 

#install libbson
RUN cd && git clone https://github.com/mongodb/libbson.git && cd libbson/build/ && cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make && make install

#install mongo-c-driver
RUN cd && git clone https://github.com/mongodb/mongo-c-driver.git && cd mongo-c-driver && sh autogen.sh && ./configure --disable-automatic-init-and-cleanup --prefix=/usr && make LDFLAGS=-pthread && make install

#install mongo-cxx-driver
RUN cd && curl -OL https://github.com/mongodb/mongo-cxx-driver/archive/r3.1.1.tar.gz && tar -xzf r3.1.1.tar.gz && echo "#include <assert.h>" > .tmp && cat /root/mongo-cxx-driver-r3.1.1/src/mongocxx/test_util/mock.hh >> .tmp && mv .tmp /root/mongo-cxx-driver-r3.1.1/src/mongocxx/test_util/mock.hh && cd mongo-cxx-driver-r3.1.1/build && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/ .. -DLIBBSON_DIR=/usr/ -DLIBMONGOC_DIR=/usr/ && make && make install

#install gdrift platform
RUN cd && git clone https://github.com/robertosolargallardo/services.git && mkdir services/build && cd services/build && cmake -DGDRIFT_ROOT_DIR=/usr -DRESTBED_ROOT_DIR=/usr -DMONGOCXX_ROOT_DIR=/usr -DBSONCXX_ROOT_DIR=/usr -DCMAKE_INSTALL_PREFIX=/usr .. && make && make install

CMD ["gdrift_service_analyzer","hosts.json"]
