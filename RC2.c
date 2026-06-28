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

void   inicializar_zonas(Zona zonas[], int n);
void   capturar_datos_actuales(Zona *z);
void   capturar_climaticos(Zona *z);
void   calcular_prediccion(Zona *z, Muestra *pred);
void   calcular_promedios(Zona *z, Muestra *prom);
void   emitir_alertas(Zona zonas[], int n);
void   mostrar_recomendaciones(Zona *z, Muestra *pred);
void   guardar_historico(Zona zonas[], int n);
void   cargar_historico(Zona zonas[], int n);
void   exportar_reporte(Zona zonas[], int n);
void   mostrar_menu(void);
void   mostrar_estado_actual(Zona zonas[], int n);
void   mostrar_predicciones(Zona zonas[], int n);
int    supera_limite(Muestra *m);
float  promedio_ponderado(float datos[], int n);
void   limpiar_buffer(void);

int main(void) {
    Zona   zonas[MAX_ZONAS];
    int    opcion, i;

    inicializar_zonas(zonas, MAX_ZONAS);
    cargar_historico(zonas, MAX_ZONAS);

    printf("\n=== SISTEMA INTEGRAL DE GESTIÓN DE CONTAMINACIÓN DEL AIRE ===\n\n");

    do {
        mostrar_menu();
        printf("Opción: ");
        scanf("%d", &opcion);
        limpiar_buffer();

        switch (opcion) {
            case 1:
                for (i = 0; i < MAX_ZONAS; i++) {
                    printf("\n--- Ingreso de datos: %s ---\n", zonas[i].nombre);
                    capturar_datos_actuales(&zonas[i]);
                    capturar_climaticos(&zonas[i]);
                }
                guardar_historico(zonas, MAX_ZONAS);
                printf("\nDatos guardados correctamente.\n");
                break;

            case 2:
                mostrar_estado_actual(zonas, MAX_ZONAS);
                break;

            case 3:
                mostrar_predicciones(zonas, MAX_ZONAS);
                break;

            case 4:
                emitir_alertas(zonas, MAX_ZONAS);
                break;

            case 5:
                exportar_reporte(zonas, MAX_ZONAS);
                printf("\nReporte exportado a '%s'.\n", ARCHIVO_REPORTE);
                break;

            case 0:
                printf("\nSistema cerrado. ¡Hasta pronto!\n");
                break;

            default:
                printf("Opción inválida.\n");
        }
    } while (opcion != 0);

    return 0;
}
void inicializar_zonas(Zona zonas[], int n) {
    const char *nombres[MAX_ZONAS] = {
        "Zona Norte", "Zona Sur", "Zona Centro",
        "Zona Este",  "Zona Oeste"
    };
    int i;
    for (i = 0; i < n; i++) {
        strncpy(zonas[i].nombre, nombres[i], MAX_NOMBRE - 1);
        zonas[i].dias_registrados = 0;
        memset(&zonas[i].actual, 0, sizeof(Muestra));
        memset(zonas[i].historico, 0, sizeof(zonas[i].historico));
        zonas[i].temperatura = 20.0f;
        zonas[i].viento      = 10.0f;
        zonas[i].humedad     = 60.0f;
    }
}

void capturar_datos_actuales(Zona *z) {
    printf("  CO2  (ppm,  límite OMS %.0f): ", LIM_CO2);
    scanf("%f", &z->actual.co2);
    printf("  SO2  (µg/m³,límite OMS %.0f): ", LIM_SO2);
    scanf("%f", &z->actual.so2);
    printf("  NO2  (µg/m³,límite OMS %.0f): ", LIM_NO2);
    scanf("%f", &z->actual.no2);
    printf("  PM2.5(µg/m³,límite OMS %.0f): ", LIM_PM25);
    scanf("%f", &z->actual.pm25);
    limpiar_buffer();

    if (z->dias_registrados < MAX_DIAS) {
        z->historico[z->dias_registrados] = z->actual;
        z->dias_registrados++;
    } else {
        int i;
        for (i = 0; i < MAX_DIAS - 1; i++)
            z->historico[i] = z->historico[i + 1];
        z->historico[MAX_DIAS - 1] = z->actual;
    }
}

void capturar_climaticos(Zona *z) {
    printf("  Temperatura (°C)  : ");  scanf("%f", &z->temperatura);
    printf("  Velocidad viento (km/h): "); scanf("%f", &z->viento);
    printf("  Humedad (%%)       : ");  scanf("%f", &z->humedad);
    limpiar_buffer();
}