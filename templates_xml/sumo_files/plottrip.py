import pandas as pd
import matplotlib.pyplot as plt
import argparse

# Configuración de los argumentos de la línea de comandos
parser = argparse.ArgumentParser(description='Graficar datos desde un archivo CSV')
parser.add_argument('-i', '--input', type=str, help='Nombre del archivo CSV de entrada', required=True)
parser.add_argument('--columnas', type=str, help='Nombres de las columnas a graficar separadas por coma', required=True)
args = parser.parse_args()

# Lee el archivo CSV
archivo_csv = args.input
datos = pd.read_csv(archivo_csv)

# Extrae las columnas de interés
tripinfo_id = datos['tripinfo_id']
columnas_seleccionadas = args.columnas.split(',')

# Graficar los datos en figuras separadas
for columna in columnas_seleccionadas:
    columna = columna.strip()  # Elimina espacios en blanco al inicio y al final
    datos_columna = datos[columna]
    
    # Crea una nueva figura para cada columna
    plt.figure(figsize=(12, 6))
    
    # Grafica datos_columna versus tripinfo_id en la nueva figura
    plt.bar(tripinfo_id, datos_columna, color='blue')
    plt.title(f'{columna} vs tripinfo_id')
    plt.xlabel('tripinfo_id')
    plt.ylabel(columna)
    plt.xticks(rotation=45, ha='right')  # Rotar las etiquetas del eje x 

    plt.tight_layout()
    plt.show()

