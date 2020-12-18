FROM ubuntu:20.04

# Prerequisites
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update && \
apt-get install -yqq dirmngr gnupg apt-transport-https ca-certificates software-properties-common && \
apt-add-repository ppa:apt-fast/stable -y && \
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9 && \
apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF && \
echo "deb https://cloud.r-project.org/bin/linux/ubuntu focal-cran40/" | tee /etc/apt/sources.list.d/r-base.list && \
echo "deb https://download.mono-project.com/repo/ubuntu vs-bionic main" | tee /etc/apt/sources.list.d/mono-official-vs.list && \
apt-get -qq update && \
apt-get -yqq install apt-fast && \
echo debconf apt-fast/maxdownloads string 16 | debconf-set-selections && \
echo debconf apt-fast/dlflag boolean true | debconf-set-selections && \
echo debconf apt-fast/aptmanager string apt-get | debconf-set-selections && \
apt-fast -yqq upgrade
# Installing Dependencies
RUN apt-fast -yqq install python3 openjdk-13-jdk git curl wget build-essential cmake python3-jira r-base nuget nuget mono-devel mono-complete monodevelop

WORKDIR /root

ARG tag
ARG branch

RUN if [ -z "${tag}" ] || [ -z "${branch}" ]; then git clone "https://github.com/brainflow-dev/brainflow.git"; else git clone --branch "${tag}" --single-branch "https://github.com/brainflow-dev/brainflow.git"; fi

WORKDIR /root/brainflow

RUN bash ./tools/build_docker.sh

