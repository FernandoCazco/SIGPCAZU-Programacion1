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
float promedio_ponderado(float datos[], int n) {
    float suma_pond = 0.0f, suma_pesos = 0.0f;
    int i;
    for (i = 0; i < n; i++) {
        float peso = (float)(i + 1);
        suma_pond  += datos[i] * peso;
        suma_pesos += peso;
    }
    return (suma_pesos > 0.0f) ? (suma_pond / suma_pesos) : 0.0f;
}

void calcular_prediccion(Zona *z, Muestra *pred) {
    int n = z->dias_registrados;
    if (n == 0) {
        *pred = z->actual;
        return;
    }
    float co2v[MAX_DIAS], so2v[MAX_DIAS];
    float no2v[MAX_DIAS], pm25v[MAX_DIAS];
    int i;
    for (i = 0; i < n; i++) {
        co2v[i]  = z->historico[i].co2;
        so2v[i]  = z->historico[i].so2;
        no2v[i]  = z->historico[i].no2;
        pm25v[i] = z->historico[i].pm25;
    }

    float f_viento  = 1.0f - (z->viento / 200.0f);
    float f_humedad = 1.0f + (z->humedad / 500.0f);
    if (f_viento  < 0.7f) f_viento  = 0.7f;
    if (f_humedad > 1.3f) f_humedad = 1.3f;

    pred->co2  = promedio_ponderado(co2v,  n) * f_viento;
    pred->so2  = promedio_ponderado(so2v,  n) * f_viento;
    pred->no2  = promedio_ponderado(no2v,  n) * f_viento;
    pred->pm25 = promedio_ponderado(pm25v, n) * f_humedad;
}

void calcular_promedios(Zona *z, Muestra *prom) {
    int n = z->dias_registrados;
    if (n == 0) { *prom = z->actual; return; }
    float sc = 0, ss = 0, sn = 0, sp = 0;
    int i;
    for (i = 0; i < n; i++) {
        sc += z->historico[i].co2;
        ss += z->historico[i].so2;
        sn += z->historico[i].no2;
        sp += z->historico[i].pm25;
    }
    prom->co2  = sc / n;
    prom->so2  = ss / n;
    prom->no2  = sn / n;
    prom->pm25 = sp / n;
}

int supera_limite(Muestra *m) {
    return (m->co2  > LIM_CO2  ||
            m->so2  > LIM_SO2  ||
            m->no2  > LIM_NO2  ||
            m->pm25 > LIM_PM25);
}
void emitir_alertas(Zona zonas[], int n) {
    int i, alerta = 0;
    Muestra pred;
    printf("\n============= ALERTAS PREVENTIVAS =============\n");
    for (i = 0; i < n; i++) {
        calcular_prediccion(&zonas[i], &pred);
        if (supera_limite(&zonas[i].actual) || supera_limite(&pred)) {
            printf("\n⚠ ALERTA – %s\n", zonas[i].nombre);
            if (zonas[i].actual.co2  > LIM_CO2)
                printf("  CO2 actual %.1f supera límite OMS (%.0f ppm)\n",
                       zonas[i].actual.co2, LIM_CO2);
            if (zonas[i].actual.so2  > LIM_SO2)
                printf("  SO2 actual %.1f supera límite OMS (%.0f µg/m³)\n",
                       zonas[i].actual.so2, LIM_SO2);
            if (zonas[i].actual.no2  > LIM_NO2)
                printf("  NO2 actual %.1f supera límite OMS (%.0f µg/m³)\n",
                       zonas[i].actual.no2, LIM_NO2);
            if (zonas[i].actual.pm25 > LIM_PM25)
                printf("  PM2.5 actual %.1f supera límite OMS (%.0f µg/m³)\n",
                       zonas[i].actual.pm25, LIM_PM25);
            if (supera_limite(&pred))
                printf("  PREDICCIÓN 24h: niveles seguirán siendo elevados.\n");
            mostrar_recomendaciones(&zonas[i], &pred);
            alerta = 1;
        }
    }
    if (!alerta)
        printf("  Sin alertas activas. Todos los niveles dentro de los límites.\n");
    printf("===============================================\n");
}

void mostrar_recomendaciones(Zona *z, Muestra *pred) {
    printf("  Recomendaciones:\n");
    if (pred->co2 > LIM_CO2)
        printf("    - Restringir circulación vehicular en %s.\n", z->nombre);
    if (pred->so2 > LIM_SO2)
        printf("    - Reducir operaciones industriales temporalmente.\n");
    if (pred->no2 > LIM_NO2)
        printf("    - Promover uso de transporte público y bicicletas.\n");
    if (pred->pm25 > LIM_PM25)
        printf("    - Suspender actividades al aire libre y usar mascarillas.\n");
    if (z->viento < 5.0f)
        printf("    - Viento bajo: menor dispersión de contaminantes.\n");
    if (z->humedad > 80.0f)
        printf("    - Alta humedad: mayor concentración de PM2.5 posible.\n");
}

void mostrar_estado_actual(Zona zonas[], int n) {
    int i;
    Muestra prom;
    printf("\n======= ESTADO ACTUAL DE CONTAMINACIÓN =======\n");
    printf("%-12s %8s %8s %8s %8s\n", "Zona", "CO2", "SO2", "NO2", "PM2.5");
    printf("%-12s %8s %8s %8s %8s\n", "----", "---", "---", "---", "-----");
    for (i = 0; i < n; i++) {
        calcular_promedios(&zonas[i], &prom);
        printf("%-12s %7.1f%c %7.1f%c %7.1f%c %7.1f%c\n",
               zonas[i].nombre,
               zonas[i].actual.co2,  zonas[i].actual.co2  > LIM_CO2  ? '!' : ' ',
               zonas[i].actual.so2,  zonas[i].actual.so2  > LIM_SO2  ? '!' : ' ',
               zonas[i].actual.no2,  zonas[i].actual.no2  > LIM_NO2  ? '!' : ' ',
               zonas[i].actual.pm25, zonas[i].actual.pm25 > LIM_PM25 ? '!' : ' ');
    }
    printf("\nLímites OMS: CO2=%.0f ppm | SO2=%.0f | NO2=%.0f | PM2.5=%.0f µg/m³\n",
           LIM_CO2, LIM_SO2, LIM_NO2, LIM_PM25);
    printf("'!' = supera límite\n");
}

void mostrar_predicciones(Zona zonas[], int n) {
    int i;
    Muestra pred;
    printf("\n======= PREDICCIÓN PRÓXIMAS 24 HORAS =======\n");
    printf("%-12s %8s %8s %8s %8s\n", "Zona", "CO2", "SO2", "NO2", "PM2.5");
    printf("%-12s %8s %8s %8s %8s\n", "----", "---", "---", "---", "-----");
    for (i = 0; i < n; i++) {
        calcular_prediccion(&zonas[i], &pred);
        printf("%-12s %7.1f  %7.1f  %7.1f  %7.1f\n",
               zonas[i].nombre,
               pred.co2, pred.so2, pred.no2, pred.pm25);
    }
}
void guardar_historico(Zona zonas[], int n) {
    FILE *f = fopen(ARCHIVO_DATOS, "wb");
    if (!f) { perror("Error al guardar"); return; }
    fwrite(zonas, sizeof(Zona), n, f);
    fclose(f);
}

void cargar_historico(Zona zonas[], int n) {
    FILE *f = fopen(ARCHIVO_DATOS, "rb");
    if (!f) return;
    fread(zonas, sizeof(Zona), n, f);
    fclose(f);
}

void exportar_reporte(Zona zonas[], int n) {
    FILE *f = fopen(ARCHIVO_REPORTE, "w");
    if (!f) { perror("Error al exportar"); return; }

    time_t t = time(NULL);
    fprintf(f, "REPORTE DE CONTAMINACIÓN DEL AIRE\n");
    fprintf(f, "Generado: %s\n", ctime(&t));
    fprintf(f, "%-12s %8s %8s %8s %8s | %8s %8s %8s %8s\n",
            "Zona", "CO2act", "SO2act", "NO2act", "PM25act",
            "CO2pred", "SO2pred", "NO2pred", "PM25pred");
    fprintf(f, "%s\n", "------------------------------------------------------------");

    int i;
    Muestra pred, prom;
    for (i = 0; i < n; i++) {
        calcular_prediccion(&zonas[i], &pred);
        calcular_promedios(&zonas[i], &prom);
        fprintf(f, "%-12s %8.1f %8.1f %8.1f %8.1f | %8.1f %8.1f %8.1f %8.1f\n",
                zonas[i].nombre,
                zonas[i].actual.co2, zonas[i].actual.so2,
                zonas[i].actual.no2, zonas[i].actual.pm25,
                pred.co2, pred.so2, pred.no2, pred.pm25);
        fprintf(f, "  Prom 30 días: CO2=%.1f SO2=%.1f NO2=%.1f PM2.5=%.1f\n",
                prom.co2, prom.so2, prom.no2, prom.pm25);
        fprintf(f, "  Clima: Temp=%.1f°C  Viento=%.1fkm/h  Humedad=%.1f%%\n\n",
                zonas[i].temperatura, zonas[i].viento, zonas[i].humedad);
    }
    fclose(f);
}

void mostrar_menu(void) {
    printf("\n--- MENÚ PRINCIPAL ---\n");
    printf(" 1. Ingresar datos de contaminación\n");
    printf(" 2. Ver estado actual de zonas\n");
    printf(" 3. Ver predicciones 24 horas\n");
    printf(" 4. Verificar alertas preventivas\n");
    printf(" 5. Exportar reporte\n");
    printf(" 0. Salir\n");
}

void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}