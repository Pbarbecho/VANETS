#!/bin/bash
# Arranca el display virtual
Xvfb :1 -screen 0 1920x1080x24 -ac &
XVFB_PID=$!

# Espera a que Xvfb esté listo
sleep 1

# Arranca Openbox (gestor de ventanas: permite mover, redimensionar, maximizar)
openbox &

# Arranca el servidor VNC sin contraseña, compartido y persistente
x11vnc -display :1 -nopw -forever -shared -rfbport 5901 -bg -o /var/log/x11vnc.log

echo " VNC disponible en  vnc://localhost:5901"
echo "   vnc://localhost:5901"
exec "$@"
