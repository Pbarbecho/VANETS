#!/bin/bash
set -e

# Crear XDG_RUNTIME_DIR antes de que dbus o Qt lo necesiten
mkdir -p /tmp/runtime-root
chmod 700 /tmp/runtime-root

# Display virtual
Xvfb :1 -screen 0 1920x1080x24 -ac +extension GLX +render -noreset &
XVFB_PID=$!

# Espera a que Xvfb esté listo (polling más robusto que sleep fijo)
for i in $(seq 1 20); do
    xdpyinfo -display :1 >/dev/null 2>&1 && break
    sleep 0.3
done

export DISPLAY=:1

# dbus (requerido por Eclipse IDE / SWT)
mkdir -p /var/run/dbus
dbus-daemon --system --fork 2>/dev/null || true
eval $(dbus-launch --sh-syntax) 2>/dev/null || true

# Gestor de ventanas
openbox &

# Servidor VNC
x11vnc -display :1 -nopw -forever -shared -rfbport 5901 -bg -o /var/log/x11vnc.log

echo "VNC disponible en vnc://localhost:5901"
exec "$@"
