# VANETS — OMNeT++ 6.1 + SUMO 1.22 Docker Environment

Entorno de simulación de redes vehiculares (VANETs) basado en contenedores Docker. Incluye **OMNeT++ 6.1.0** compilado desde fuente y **SUMO 1.22.0**, con interfaz gráfica accesible vía VNC.

---

## Estructura del repositorio

```
.
├── dockerfile          # Imagen para macOS (linux/amd64 via Rosetta/platform flag)
├── entrypoint.sh       # Script de inicio: Xvfb + VNC + shell
├── mac/
│   ├── dockerfile      # Dockerfile específico para macOS (Apple Silicon / Intel)
│   └── entrypoint.sh
└── windows/
    ├── dockerfile      # Dockerfile específico para Windows (x86_64)
    └── entrypoint.sh
```

---

## Cómo funciona

### 1. Dockerfile

El `dockerfile` construye una imagen Ubuntu 22.04 que contiene:

| Componente | Versión | Instalación |
|---|---|---|
| OMNeT++ | 6.1.0 | Compilado desde fuente (`make -j$(nproc)`) |
| SUMO | 1.22.0 | `pip install eclipse-sumo` (binarios precompilados) |
| Qt | 5 + 6 | Paquetes apt (backend xcb para Xvfb) |
| Python | 3.10 | numpy, pandas, matplotlib, scipy, ipython |
| Xvfb | — | Display virtual `:1` en 1920×1080×24 |
| x11vnc | — | Servidor VNC sin contraseña en puerto **5901** |
| Openbox | — | Gestor de ventanas ligero |

### 2. entrypoint.sh

Al iniciar el contenedor, el entrypoint ejecuta en orden:

1. Crea `XDG_RUNTIME_DIR` (`/tmp/runtime-root`)
2. Lanza **Xvfb** en el display `:1` (1920×1080×24 bits, con extensión GLX)
3. Espera a que Xvfb esté listo (polling con `xdpyinfo`)
4. Inicia **dbus** (requerido por el IDE Eclipse/SWT de OMNeT++)
5. Lanza **Openbox** (gestor de ventanas)
6. Inicia **x11vnc** escuchando en el puerto `5901`
7. Ejecuta el comando pasado al contenedor (por defecto `/bin/bash`)

La GUI queda accesible en `vnc://localhost:5901` desde el host.

---

## Prerrequisitos

### Docker

- [Docker Desktop](https://www.docker.com/products/docker-desktop/) instalado y en ejecución.

### Visor VNC

Necesitas un cliente VNC instalado en tu máquina host para ver la interfaz gráfica.

#### macOS

| Opción | Notas |
|---|---|
| **Screen Sharing** (nativo) | Abre Finder → `Cmd+K` → `vnc://localhost:5901`. Sin instalación extra. |
| [RealVNC Viewer](https://www.realvnc.com/en/connect/download/viewer/) | Gratuito, multiplataforma. |
| [TigerVNC](https://tigervnc.org/) | `brew install tiger-vnc` |

#### Windows

| Opción | Notas |
|---|---|
| [RealVNC Viewer](https://www.realvnc.com/en/connect/download/viewer/) | Recomendado. Gratuito. |
| [TigerVNC](https://tigervnc.org/) | Instalador `.exe` disponible en GitHub Releases. |
| [TightVNC](https://www.tightvnc.com/) | Alternativa ligera. |

---

## Construir la imagen

### macOS (Apple Silicon M1/M2/M3 o Intel)

El dockerfile de macOS usa `--platform=linux/amd64` para garantizar compatibilidad con los binarios de OMNeT++ y SUMO.

```bash
# Desde la raíz del repositorio (usa el dockerfile de mac/)
docker build --platform linux/amd64 -t vanets:mac -f mac/dockerfile mac/
```

> En Apple Silicon, Docker Desktop ejecuta la imagen x86_64 vía Rosetta 2 de forma transparente. Asegúrate de tener habilitada la opción **"Use Rosetta for x86/amd64 emulation"** en Docker Desktop → Settings → General.

### Windows

```bat
:: Desde la raíz del repositorio (usa el dockerfile de windows/)
docker build -t vanets:windows -f windows\dockerfile windows\
```

---

## Lanzar el contenedor

### macOS

```bash
docker run -it --rm \
  --platform linux/amd64 \
  -p 5901:5901 \
  vanets:mac
```

Una vez iniciado, conecta tu visor VNC a `vnc://localhost:5901`.

**Con directorio compartido** (para montar tus proyectos de simulación):

```bash
docker run -it --rm \
  --platform linux/amd64 \
  -p 5901:5901 \
  -v "$PWD/simulations":/root/simulations \
  vanets:mac
```

### Windows

En **PowerShell** o **CMD**:

```powershell
docker run -it --rm `
  -p 5901:5901 `
  vanets:windows
```

```bat
docker run -it --rm ^
  -p 5901:5901 ^
  vanets:windows
```

**Con directorio compartido:**

```powershell
docker run -it --rm `
  -p 5901:5901 `
  -v "${PWD}\simulations:/root/simulations" `
  vanets:windows
```

Una vez iniciado, abre RealVNC Viewer (u otro cliente) y conéctate a `localhost:5901`.

---

## Verificar que el entorno funciona

Dentro del contenedor (en la terminal que aparece tras el `docker run`):

```bash
# Verificar SUMO
sumo --version

# Verificar OMNeT++
opp_run --version

# Abrir el IDE de OMNeT++ (visible en el visor VNC)
omnetpp
```

---

## Variables de entorno relevantes

| Variable | Valor | Descripción |
|---|---|---|
| `SUMO_HOME` | `/usr/local/lib/python3.10/dist-packages/sumo` | Ruta base de SUMO |
| `DISPLAY` | `:1` | Display virtual Xvfb |
| `QT_QPA_PLATFORM` | `xcb` | Backend Qt (evita fallos con Wayland) |
| `LIBGL_ALWAYS_SOFTWARE` | `1` | Mesa renderiza en software (sin GPU física) |
| `GALLIUM_DRIVER` | `llvmpipe` | Driver de renderizado por software |

---

## Notas

- El servidor VNC arranca **sin contraseña**. No expongas el puerto 5901 en redes públicas.
- La compilación de OMNeT++ desde fuente tarda varios minutos. La imagen final pesa aproximadamente 8–10 GB.
- Si necesitas persistir datos entre sesiones, usa un volumen Docker (`-v`) o evita `--rm`.
