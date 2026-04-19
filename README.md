# Laboratorio de Redes Vehiculares (VANETs)

Entorno de simulación contenerizado para el estudio y experimentación con **Redes Vehiculares Ad Hoc (VANETs)**, integrando SUMO, OMNeT++ y Veins.

---

## Descripción

Este repositorio proporciona los recursos necesarios para configurar y ejecutar simulaciones de redes vehiculares, incluyendo:

- Un entorno Docker preconfigurado con todas las dependencias necesarias.
- Scripts de lanzamiento para la integración SUMO–Veins–OMNeT++.
- Guía de laboratorio y material de apoyo para el aprendizaje de VANETs.

---

## Herramientas utilizadas

| Herramienta | Versión | Descripción |
|---|---|---|
| [SUMO](https://eclipse.dev/sumo/) | 1.22 | Simulador de movilidad urbana |
| [OMNeT++](https://omnetpp.org/) | 6.3.0 | Framework de simulación de redes |
| [Veins](https://veins.car2x.org/) | 5.3.1 | Framework de simulación de redes vehiculares |
| Docker | — | Contenerización del entorno |
| Ubuntu | 22.04 | Sistema operativo base del contenedor |

---

## Contenido del repositorio

```
VANETS/
├── dockerfile                          # Imagen Docker con SUMO + OMNeT++ + Veins
├── run_sumo.sh                         # Script para lanzar el servidor SUMO (Veins TraCI)
├── Guia_Laboratorio_Redes_Vehiculares.pdf  # Guía de laboratorio completa
└── Slides_SUMO.pdf                     # Presentación introductoria a SUMO
```

---

## Requisitos previos

- [Docker](https://docs.docker.com/get-docker/) instalado en el sistema anfitrión.
- Servidor X11 para visualización de interfaces gráficas:
  - **Linux:** `xorg` / `xserver` (disponible por defecto).
  - **macOS:** [XQuartz](https://www.xquartz.org/).
  - **Windows:** [VcXsrv](https://sourceforge.net/projects/vcxsrv/) o [X410](https://x410.dev/).

---

## Instalación y uso

### 1. Clonar el repositorio

```bash
git clone https://github.com/Pbarbecho/VANETS.git
cd VANETS
```

### 2. Construir la imagen Docker

> Este proceso descarga y compila SUMO desde el código fuente, por lo que puede tardar varios minutos.

```bash
docker build -t vanets-env .
```

### 3. Habilitar reenvío de GUI (X11)

**Linux:**
```bash
xhost +local:docker
```

**macOS (XQuartz):**
```bash
xhost + 127.0.0.1
```

### 4. Ejecutar el contenedor

**Linux:**
```bash
docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  vanets-env
```

**macOS:**
```bash
docker run -it --rm \
  -e DISPLAY=host.docker.internal:0 \
  vanets-env
```

### 5. Lanzar el servidor SUMO (dentro del contenedor)

El script `run_sumo.sh` inicia el demonio `sumo-launchd`, que actúa como puente de comunicación TraCI entre SUMO y OMNeT++/Veins:

```bash
bash run_sumo.sh
```

Esto ejecuta:
```bash
/root/omnet/veins-veins-5.3.1/sumo-launchd.py -vv -c /usr/share/sumo/bin/sumo
```

---

## Arquitectura de la simulación

```
┌─────────────────────┐        TraCI         ┌──────────────────────┐
│   OMNeT++ + Veins   │ ◄──────────────────► │        SUMO          │
│  (simulación de red)│    sumo-launchd.py   │  (movilidad urbana)  │
└─────────────────────┘                       └──────────────────────┘
         │
         └── Modelos de comunicación V2V / V2I (802.11p / DSRC)
```

- **SUMO** genera la movilidad de los vehículos (trayectorias, velocidades, posiciones).
- **Veins** actúa como middleware, sincronizando SUMO con OMNeT++ a través del protocolo TraCI.
- **OMNeT++** simula el comportamiento de la red de comunicaciones entre vehículos.

---

## Variables de entorno del contenedor

| Variable | Valor | Descripción |
|---|---|---|
| `SUMO_HOME` | `/opt/sumo` | Directorio raíz de SUMO |
| `PATH` | `$SUMO_HOME/bin:$PATH` | Acceso directo a binarios de SUMO |
| `PYTHONPATH` | `$SUMO_HOME/tools` | Herramientas Python de SUMO |
| `DISPLAY` | `host.docker.internal:0` | Reenvío de GUI al anfitrión |
| `LIBGL_ALWAYS_SOFTWARE` | `1` | Renderizado OpenGL por software |

---

## Material de apoyo

| Archivo | Descripción |
|---|---|
| `Guia_Laboratorio_Redes_Vehiculares.pdf` | Guía paso a paso de los laboratorios de VANETs |
| `Slides_SUMO.pdf` | Introducción teórico-práctica al simulador SUMO |

---

## Referencias

- [SUMO Documentation](https://sumo.dlr.de/docs/)
- [OMNeT++ Documentation](https://doc.omnetpp.org/)
- [Veins Documentation](https://veins.car2x.org/documentation/)
- [Veins Tutorial](https://veins.car2x.org/tutorial/)
- [TraCI (Traffic Control Interface)](https://sumo.dlr.de/docs/TraCI.html)
