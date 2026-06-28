#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_ZONAS       5
#define MAX_DIAS        30
#define MAX_NOMBRE      50
#define ARCHIVO_DATOS   "historico.dat"
#define ARCHIVO_REPORTE "reporte.txt"

#define LIM_CO2   1000.0
#define LIM_SO2     20.0
#define LIM_NO2     25.0
#define LIM_PM25    15.0

typedef struct {
    float co2;
    float so2;
    float no2;
    float pm25;
} Muestra;

typedef struct {
    char  nombre[MAX_NOMBRE];
    Muestra historico[MAX_DIAS];
    int   dias_registrados;
    Muestra actual;
    float temperatura;
    float viento;
    float humedad;
} Zona;