FROM espressif/idf:release-v4.4
LABEL Name=linefollowerbraiacode Version=0.0.1

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

ARG DEBIAN_FRONTEND=nointeractive
RUN apt-get update \
    && apt-get -qqy install \
        # Common stuff
        apt-utils \
        build-essential \
        ca-certificates \
        curl \
        dialog \
        git \
        sudo \
        unzip \
        wget \
        # for platformio
        bc \
        build-essential \
        clang \
        curl \
        gcc \
        python3 \
        python3-dev \
        python3-distutils \
        python3-pip \
        srecord \
        xz-utils \
        # for QEMU
        cmake \
        libglib2.0-0 \
        libnuma1 \
        libpixman-1-0 \
    # Clean up
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN if [ ${USER_UID:-0} -ne 0 ] && [ ${USER_GID:-0} -ne 0 ]; then \
        groupadd --gid $USER_GID $USERNAME \
        && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME \
        && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME\
        && chmod 0440 /etc/sudoers.d/$USERNAME \
    ; fi

# QEMU
ENV QEMU_REL=esp-develop-20220203
ENV QEMU_SHA256=C83E483E3290F48A563C2A376B7413CD94A8692D8C7308B119F4268CA6D164B6
ENV QEMU_DIST=qemu-${QEMU_REL}.tar.bz2
ENV QEMU_URL=https://github.com/espressif/qemu/releases/download/${QEMU_REL}/${QEMU_DIST}

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV IDF_PYTHON_ENV_PATH=/opt/esp/python_env/idf4.4_py3.8_env

RUN wget --no-verbose ${QEMU_URL} \
  && echo "${QEMU_SHA256} *${QEMU_DIST}" | sha256sum --check --strict - \
  && tar -xf $QEMU_DIST -C /opt \
  && rm ${QEMU_DIST}

ENV PATH=/opt/qemu/bin:${PATH}

# https://docs.platformio.org/en/latest/faq.html#platformio-udev-rules
RUN curl -fLo /etc/udev/rules.d/99-platformio-udev.rules --create-dirs https://raw.githubusercontent.com/platformio/platformio-core/master/scripts/99-platformio-udev.rules \
    && usermod -a -G dialout $USERNAME \
    && usermod -a -G plugdev $USERNAME

USER $USERNAME
RUN python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py)"

RUN echo $($IDF_PATH/tools/idf_tools.py export) >> $HOME/.bashrc

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash"]