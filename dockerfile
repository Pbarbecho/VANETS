
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 1. Herramientas de sistema, compiladores y dependencias gráficas
RUN apt-get update && apt-get install -y \
    nano git make diffutils pkg-config curl ccache clang lld gdb lldb \
    sed gawk python3-venv python3-dev \
    libxml2-dev zlib1g-dev doxygen graphviz libdw-dev \
    build-essential clang bison flex perl python3 python3-pip \
    libxml2-dev zlib1g-dev default-jre curl ca-certificates \
    libqt5gui5 libqt5opengl5-dev libqt5svg5-dev \
    qt6-base-dev qt6-base-dev-tools qmake6 libqt6svg6 \
    libqt6gui6 libqt6opengl6-dev \
    qt6-wayland libwebkit2gtk-4.1-0 libgl1-mesa-dev libglu1-mesa-dev \
    libtk8.6 blt xdg-utils libopenscenegraph-dev \
    libgl1-mesa-dri libgl1-mesa-glx mesa-utils libosmesa6 \
    nemiver mpi-default-dev \
    libwebkit2gtk-4.1-0 xdg-utils \
    libfox-1.6-0 libgdal30 libproj22 libxerces-c3.2 \
    sumo sumo-tools sumo-doc \
    && rm -rf /var/lib/apt/lists/*

# 2. Módulos de Python necesarios para OMNeT++ 6.3.0
RUN pip3 install --upgrade pip setuptools && \
    pip3 install \
    "packaging>=23.0.0" \
    "matplotlib>=3.5.2,<4.0.0" \
    "numpy>=1.18.0,<3.0.0" \
    "pandas>=1.0.0,<3.0.0" \
    "scipy>=1.0.0,<2.0.0" \
    "ipython>=7.0.0" \
    posix_ipc wheel opp_env

# 3. Salta el error de xdg-desktop-menu en Docker
RUN ln -sf /bin/true /usr/local/bin/xdg-desktop-menu && \
    ln -sf /bin/true /usr/local/bin/xdg-icon-resource && \
    ln -sf /bin/true /usr/local/bin/xdg-mime

# 4. Variables de Entorno
# Configuración de Variables de Entorno para SUMO y Veins
ENV SUMO_HOME=/usr/share/sumo
#ENV PATH="/opt/sumo/bin:${PATH}"
#ENV LD_LIBRARY_PATH="/opt/sumo/bin"

# Estas variables son globales y aseguran la estabilidad gráfica
ENV OPP_ENV_USE_NIX=no
ENV LIBGL_ALWAYS_SOFTWARE=1
ENV QT_X11_NO_MITSHM=1
ENV DISPLAY=host.docker.internal:0
ENV XDG_RUNTIME_DIR=/tmp/runtime-root

WORKDIR /root/omnet

# 5. Configuración del Shell para carga automática
# Creamos el .bashrc si no existe y añadimos la activación protegida
RUN touch /root/.bashrc && \
    echo 'if [ -d "/root/omnet/.opp_env" ]; then source /root/omnet/.opp_env/bin/activate; fi' >> /root/.bashrc

CMD ["/bin/bash"]
