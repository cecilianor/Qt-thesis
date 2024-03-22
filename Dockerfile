# This is the Docker script to generate
# a Docker image that contains a precompiled version of 
# Qt from source.
#
# This should be ran once on a local machine and 
# then uploaded to a Docker container repository,
# which can the be used in the GitHub Actions.

FROM ubuntu:22.04

# Install build dependencies for Qt
RUN apt-get update && apt-get install -y \
	git \
	cmake \
	g++ \
	ninja-build \
	pkg-config \
	libprotoc-dev \
	libprotobuf-dev \
	protobuf-compiler \
	protobuf-compiler-grpc \
	libgrpc-dev \
	libgrpc++-dev \
    && rm -rf /var/lib/apt/lists/*

# Clone the Qt dev branch
RUN git clone -b dev --depth 1 https://code.qt.io/qt/qt5.git qtsrc && \
    cd qtsrc && \
    ./init-repository --module-subset="qtbase,qtgrpc,qtrepotools" -f --no-optional-deps

# Configure and build Qt
RUN mkdir qtbuild && \
	cd qtbuild && \
	../qtsrc/configure -developer-build -nomake examples -nomake tests -no-opengl -skip qtdeclarative && \
	ninja

# Cleanup to reduce image size
RUN apt-get clean && rm -rf /var/lib/apt/lists/*