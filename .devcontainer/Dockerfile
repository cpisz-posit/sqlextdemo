# This file is currently referenced by devcontainer.json and provides the instructions that builds the container in docker.
# VSCode or another IDE should be able to build the dev container using that json file.
# If you do not wish to use dev containers you can manually create a container using the following instructions:
#
# To build the docker image:
# docker build -t devenvcpp/ubuntu:0.1 .
#
# To create the docker container and open the terminal
# This will create a volume for us to persist our files. We can mount this volume in other containers later if we like.
#   The volume differs from mounting a directory from the host OS, in that docker will manage it and we won't access it from the host. 
#   That way we don't have to worry about line endings and such.
# docker run --name devenvcpp --user developer -it --mount type=volume,src=vol_devenvcpp,dst=/home/developer/volume devenvcpp/ubuntu:0.1 bash
# 
# When done, just 'exit'
# 
# To start the already created docker container and open the terminal
# docker start --attach --interactive devenvcpp
#
# When done, just 'exit'

FROM ubuntu:22.04
SHELL ["/bin/bash", "-c"]
LABEL Description="Chris' Ubuntu based development environment for C++"

ARG DEBIAN_FRONTEND=noninteractive
ARG USERNAME=developer
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Install all the things we'll need
#    I am installing git on the container rather than the host, because we are using the volume rather than mounting a directory we share with the host
#    in my opinion, doing the latter invites all kinds of problems.
RUN apt update && apt -y --no-install-recommends install \
    sudo \
    bash \
    vim \
    net-tools \
    git \
    ssh \
    build-essential \
    clang \
    cmake \
    ninja-build \
    gdb \
    wget \
    python3 \
    python3-pip \
    libsqlite3-dev \
    postgresql \
    libpq-dev

# Boost 1.83.0
RUN cd /tmp && \
    wget https://archives.boost.io/release/1.83.0/source/boost_1_83_0.tar.gz && \
    tar -xvf boost_1_83_0.tar.gz && \
    cd boost_1_83_0 && \
    ./bootstrap.sh && \
    ./b2 install

# Soci    
RUN cd /tmp && \
    git clone https://github.com/SOCI/soci.git && \
    cd soci && \
    git checkout release/4.0 && \
    mkdir build && \
    cd build && \
    cmake -G "Unix Makefiles" -DWITH_BOOST=ON -DWITH_SQLITE3=ON -DSOCI_SQLITE3=ON -DWITH_POSTGRESQL=ON -DSOCI_POSTGRESQL=ON .. && \
    make && \
    make install

# Package manager for C++ is what everyone needs in thier life
# Note - We could of gotten boost and soci with conan via a conan file in our actual project, which is much preferred
RUN pip install conan

# Create a non-root user and give them sudo
RUN groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME && \
    echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME && \
    chmod 0440 /etc/sudoers.d/$USERNAME

# Change to the user
USER $USERNAME
WORKDIR /home/$USERNAME

# As the user, make the directory we will map the volume to, as the user, so it has proper permissions
RUN mkdir -p /home/$USERNAME/volume
