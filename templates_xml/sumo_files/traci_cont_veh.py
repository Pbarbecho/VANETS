import traci
import csv
import time
import re


# Iniciar la conexión
traci.start(["sumo", "-c", "mapa.sumo.cfg"])

# Lista de identificadores de vehículos
lista_veh = []

while traci.simulation.getMinExpectedNumber() > 0:
    step = traci.simulation.getTime()
    vehiculos_activos = traci.vehicle.getIDList()
    cantidad = len(vehiculos_activos)

    print(f"Tiempo: {step:.1f}s | # veh: {cantidad}")

    lista_veh.extend(vehiculos_activos)
    traci.simulationStep()

traci.close()