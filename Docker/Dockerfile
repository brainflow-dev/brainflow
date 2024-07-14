FROM ubuntu:20.04

# Prerequisites
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update && apt-get install -y wget
RUN wget https://packages.microsoft.com/config/ubuntu/20.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb 
RUN dpkg -i packages-microsoft-prod.deb
RUN apt-get -qq update && \
apt-get install -yqq dirmngr gnupg apt-transport-https ca-certificates software-properties-common && \
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9 && \
apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF && \
echo "deb https://cloud.r-project.org/bin/linux/ubuntu focal-cran40/" | tee /etc/apt/sources.list.d/r-base.list && \
echo "deb https://download.mono-project.com/repo/ubuntu vs-bionic main" | tee /etc/apt/sources.list.d/mono-official-vs.list && \
apt-get -qq update
# Installing Dependencies
RUN apt-get install -yqq python3 python3-pip python3-venv openjdk-13-jdk maven git curl wget build-essential python3-jira r-base rustc cargo nuget nuget mono-devel mono-complete monodevelop libxml2-dev libbluetooth-dev libdbus-1-dev dotnet-sdk-7.0 dotnet-runtime-7.0 dotnet-runtime-3.1 aspnetcore-runtime-7.0 nodejs npm
RUN mkdir -p /root/local/bin
WORKDIR /root/local/bin
# install latest cmake
RUN mkdir -p /opt/cmake && wget https://github.com/Kitware/CMake/releases/download/v3.24.3/cmake-3.24.3-linux-x86_64.sh && sh cmake-3.24.3-linux-x86_64.sh --prefix=/opt/cmake --skip-license
ENV PATH=/opt/cmake/bin/:$PATH
RUN wget "https://julialang-s3.julialang.org/bin/linux/x64/1.5/julia-1.5.3-linux-x86_64.tar.gz"
RUN mkdir -p julia
RUN tar xf julia-1.5.3-linux-x86_64.tar.gz -C ./julia
ENV PATH=$PATH:/root/local/bin/julia/julia-1.5.3/bin

# Install useful python packages not required by BrainFlow
RUN python3 -m pip install scipy pandas matplotlib mne jupyterlab notebook opencv-python pyriemann

WORKDIR /root
ARG checkout_id="none"
# Compile
RUN git clone "https://github.com/brainflow-dev/brainflow.git"
WORKDIR /root/brainflow
RUN if [ "${checkout_id}" != "none" ]; then git checkout "${checkout_id}"; fi
RUN python3 ./tools/build.py --use-openmp --bluetooth --ble
ENV LD_LIBRARY_PATH=/root/brainflow/installed_linux/lib/

# Python Binding
WORKDIR /root/brainflow
RUN python3 -m pip install --user virtualenv
WORKDIR /root/brainflow/python_package
RUN python3 -m pip install -U .

WORKDIR /root/brainflow/
# C# Binding
RUN nuget restore csharp_package/brainflow/brainflow.sln
RUN dotnet build csharp_package/brainflow/brainflow.sln

# Java Binding
WORKDIR /root/brainflow/java_package/brainflow
RUN mvn package

# Julia Bindings
WORKDIR /root/brainflow/julia_package/brainflow
RUN /root/local/bin/julia/julia-1.5.3/bin/julia -e "import Pkg; Pkg.instantiate()"
RUN /root/local/bin/julia/julia-1.5.3/bin/julia -e "import Pkg; Pkg.activate()"

# R Binding
WORKDIR /root/brainflow/r_package/brainflow
RUN R --vanilla -e 'install.packages("knitr", repos="http://cran.us.r-project.org")'
RUN R --vanilla -e 'install.packages("reticulate", repos="http://cran.us.r-project.org")'
RUN R CMD build .

# Node JS Binding
WORKDIR /root/brainflow/nodejs_binding/brainflow
RUN npm install -g ts-node
RUN npm install

# Rust Binding
WORKDIR /root/brainflow/rust_package/brainflow
RUN cargo build

WORKDIR /root/brainflow
