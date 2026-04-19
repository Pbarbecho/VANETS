FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# ─────────────────────────────────────────────
# 1. Herramientas de sistema base
# ─────────────────────────────────────────────
RUN apt-get update && apt-get install -y \
    build-essential gcc g++ clang lld ccache gdb lldb \
    cmake make ninja-build pkg-config \
    git curl wget ca-certificates \
    nano sed gawk diffutils \
    && rm -rf /var/lib/apt/lists/*

# ─────────────────────────────────────────────
# 2. Dependencias runtime de SUMO
#    (solo librerías de ejecución, no -dev: SUMO se instala via pip)
# ─────────────────────────────────────────────
RUN apt-get update && apt-get install -y \
    libxerces-c3.2 \
    libfox-1.6-0 \
    libfreetype6 \
    libproj22 proj-data proj-bin \
    libxml2 \
    zlib1g \
    libgdal30 \
    && rm -rf /var/lib/apt/lists/*

# ─────────────────────────────────────────────
# 3. Dependencias de OMNeT++ 6.x
#    (compilador, gráficos, MPI, Java, Python)
# ─────────────────────────────────────────────
RUN apt-get update && apt-get install -y \
    bison flex perl \
    default-jre \
    python3 python3-dev python3-pip python3-venv \
    libqt5gui5 libqt5opengl5-dev libqt5svg5-dev qtbase5-dev qtchooser \
    qt6-base-dev qt6-base-dev-tools qmake6 \
    libqt6svg6 libqt6gui6 libqt6opengl6-dev \
    qt6-wayland \
    libgl1-mesa-dev libglu1-mesa-dev \
    libgl1-mesa-dri libgl1-mesa-glx mesa-utils libosmesa6 \
    libopenscenegraph-dev \
    mpi-default-dev \
    doxygen graphviz \
    libtk8.6 blt xdg-utils \
    libwebkit2gtk-4.1-0 \
    nemiver \
    # ── X11 client + display virtual + VNC ──────
    xauth libx11-dev libxext-dev libxrender-dev \
    libxrandr-dev libxi-dev libxtst-dev \
    x11-apps \
    xvfb x11vnc \
    openbox \
    && rm -rf /var/lib/apt/lists/*

# ─────────────────────────────────────────────
# 4. Paquetes Python para OMNeT++ / SUMO tools
# ─────────────────────────────────────────────
RUN pip3 install --upgrade pip setuptools wheel && \
    pip3 install \
    "packaging>=23.0.0" \
    "matplotlib>=3.5.2,<4.0.0" \
    "numpy>=1.18.0,<2.0.0" \
    "pandas>=1.0.0,<3.0.0" \
    "scipy>=1.0.0,<2.0.0" \
    "ipython>=7.0.0" \
    posix_ipc

# ─────────────────────────────────────────────
# 5. Instalar SUMO v1.15.0 via pip (binarios precompilados)
#    Más rápido que compilar desde fuente y versión estable oficial
# ─────────────────────────────────────────────
RUN pip3 install eclipse-sumo==1.22.0

# ─────────────────────────────────────────────
# 6. Variables de entorno
# ─────────────────────────────────────────────
ENV SUMO_HOME=/usr/local/lib/python3.10/dist-packages/sumo
ENV PATH=$SUMO_HOME/bin:$PATH
ENV PYTHONPATH=$SUMO_HOME/tools

ENV OPP_ENV_USE_NIX=no
# Mesa renderiza localmente sobre Xvfb — sin GLX remoto, sin GLXBadContext
ENV LIBGL_ALWAYS_SOFTWARE=1
ENV GALLIUM_DRIVER=llvmpipe
ENV QT_X11_NO_MITSHM=1
# Display virtual interno (Xvfb), expuesto via VNC en el puerto 5901
ENV DISPLAY=:1
ENV XDG_RUNTIME_DIR=/tmp/runtime-root

# ─────────────────────────────────────────────
# 7. Sustituir comandos xdg que fallan en Docker
# ─────────────────────────────────────────────
RUN ln -sf /bin/true /usr/local/bin/xdg-desktop-menu && \
    ln -sf /bin/true /usr/local/bin/xdg-icon-resource && \
    ln -sf /bin/true /usr/local/bin/xdg-mime

# Menú mínimo de Openbox (evita: Unable to find a valid menu file)
RUN mkdir -p /var/lib/openbox && \
    printf '<?xml version="1.0" encoding="UTF-8"?>\n<openbox_menu xmlns="http://openbox.org/3.4/menu"></openbox_menu>\n' \
    > /var/lib/openbox/debian-menu.xml

# Reemplazar dash por bash como /bin/sh
# (evita "Bad substitution" en scripts de OMNeT++ que usan sintaxis bash)
RUN echo "dash dash/sh boolean false" | debconf-set-selections && \
    dpkg-reconfigure -f noninteractive dash

# ─────────────────────────────────────────────
# 8. Instalar OMNeT++ 6.1.0 desde fuente (sin opp_env/Nix)
# ─────────────────────────────────────────────
SHELL ["/bin/bash", "-c"]

WORKDIR /root/omnet

RUN wget --no-verbose \
        https://github.com/omnetpp/omnetpp/releases/download/omnetpp-6.1.0/omnetpp-6.1.0-linux-x86_64.tgz \
        -O /tmp/omnetpp.tgz && \
    tar -xzf /tmp/omnetpp.tgz -C /root/omnet && \
    rm /tmp/omnetpp.tgz

WORKDIR /root/omnet/omnetpp-6.1

# Configurar y compilar OMNeT++
RUN . ./setenv && \
    ./configure && \
    make -j$(nproc)

# Exponer binarios de OMNeT++ en el PATH
RUN touch /root/.bashrc && \
    echo '. /root/omnet/omnetpp-6.1/setenv' >> /root/.bashrc

ENV PATH=/root/omnet/omnetpp-6.1/bin:$PATH

# ─────────────────────────────────────────────
# 9. Entrypoint: arranca Xvfb + x11vnc y luego bash
# ─────────────────────────────────────────────
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 5901

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/bin/bash"]
