FROM ubuntu
ENV TZ=Europe/Moscow
RUN apt-get clean && apt-get update -y
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata git sudo build-essential autoconf libtool libssl-dev zlib1g-dev librdkafka-dev pkg-config cmake gcc git g++ mysql-client libmysqlclient-dev libboost-all-dev libssl-dev && apt-get clean
RUN git clone -b master https://github.com/pocoproject/poco.git && cd poco && mkdir cmake-build && cd cmake-build && cmake .. && make && sudo make install && cd .. && cd ..
RUN git clone https://github.com/sttell/conference_blog_service.git -b feature/lab_2 && \
    sudo ldconfig && \
    cd conference_blog_service/sources && mkdir build && cd build && cmake .. && make

ENTRYPOINT [ "/conference_blog_service/sources/build/users_service/users_service", "/conference_blog_service/sources/build/users_service/users_service_data/server_config.json" ]
