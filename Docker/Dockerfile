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
RUN apt-fast -yqq install python3 python3-pip python3-venv openjdk-13-jdk git curl wget build-essential cmake python3-jira r-base nuget nuget mono-devel mono-complete monodevelop libxml2-dev libbluetooth-dev libdbus-1-dev
RUN mkdir -p /root/local/bin

WORKDIR /root/local/bin
# Installing Maven and Julia
RUN curl "https://miroir.univ-lorraine.fr/apache/maven/maven-3/3.6.3/binaries/apache-maven-3.6.3-bin.tar.gz" -o maven.tar.gz
RUN mkdir -p maven
RUN tar xzf maven.tar.gz -C ./maven
RUN export PATH=$PATH:/root/local/bin/maven/apache-maven-3.6.3/bin
RUN wget "https://julialang-s3.julialang.org/bin/linux/x64/1.5/julia-1.5.3-linux-x86_64.tar.gz"
RUN mkdir -p julia
RUN tar xf julia-1.5.3-linux-x86_64.tar.gz -C ./julia
RUN export PATH=$PATH:/root/local/bin/julia/julia-1.5.3/bin

# Install useful python packages not required by BrainFlow
RUN python3 -m pip install scipy pandas matplotlib mne jupyterlab notebook opencv-python pyriemann

WORKDIR /root
ARG checkout_id
# Compile
RUN git clone "https://github.com/brainflow-dev/brainflow.git"
WORKDIR /root/brainflow
RUN if [ -z "${checkout_id}" ]; then git checkout "${checkout_id}"; fi
RUN python3 ./tools/build.py --use-openmp --bluetooth --ble
ENV LD_LIBRARY_PATH=/root/brainflow/installed_linux/lib/

# Python Binding
WORKDIR /root/brainflow
RUN python3 -m pip install --user virtualenv
WORKDIR /root/brainflow/python-package
RUN python3 -m pip install -U .

WORKDIR /root/brainflow/
# C# Binding
RUN nuget restore csharp-package/brainflow/brainflow.sln
RUN xbuild csharp-package/brainflow/brainflow.sln

# Java Binding
WORKDIR /root/brainflow/java-package/brainflow
RUN /root/local/bin/maven/apache-maven-3.6.3/bin/mvn package

# Julia Bindings
WORKDIR /root/brainflow/julia-package/brainflow
RUN /root/local/bin/julia/julia-1.5.3/bin/julia -e "import Pkg; Pkg.instantiate()"
RUN /root/local/bin/julia/julia-1.5.3/bin/julia -e "import Pkg; Pkg.activate()"

# R Binding
WORKDIR /root/brainflow/r-package/brainflow
RUN R --vanilla -e 'install.packages("knitr", repos="http://cran.us.r-project.org")'
RUN R --vanilla -e 'install.packages("reticulate", repos="http://cran.us.r-project.org")'
RUN R CMD build .

WORKDIR /root/brainflow
