import traci

traci.start(["sumo", "-c", "mapa.sumo.cfg"])

while traci.simulation.getMinExpectedNumber() > 0:
    traci.simulationStep()
    
traci.close()
