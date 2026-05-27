FROM ghcr.io/pgaskin/nickeltc:1

ENV CROSS_COMPILE=arm-nickel-linux-gnueabihf-

# Clone FBInk and its submodules
RUN git clone --recursive https://github.com/NiLuJe/FBInk.git /opt/FBInk

# Patch FBInk's Makefile to remove unsupported compiler flag for i2c-tools
RUN sed -i "s/EXTRA_CFLAGS+=-fno-semantic-interposition/#EXTRA_CFLAGS+=-fno-semantic-interposition/" /opt/FBInk/Makefile

# Build FBInk
RUN make -C /opt/FBInk kobo

WORKDIR /work

CMD ["make", "CC=arm-nickel-linux-gnueabihf-gcc", "FBINK_INC_DIR=/opt/FBInk", "FBINK_LIB_DIR=/opt/FBInk/Release"]
