import sys
import re
from datetime import datetime, timedelta
import os
import math

def es_linea_valida_csv(linea):
    if not linea or len(linea) < 5:
        return False
    partes = linea.split(',')
    if len(partes) < 5:
        return False
    try:
        id_num = int(partes[0])
        if id_num < 0 or id_num > 100000:
            return False
    except:
        return False
    fecha_pattern = r'\d{1,2}/\d{1,2}/\d{2,4}'
    if not re.search(fecha_pattern, partes[1]):
        return False
    hora_pattern = r'\d{1,2}:\d{2}:\d{2}'
    if not re.search(hora_pattern, partes[2]):
        return False
    return True

def limpiar_y_convertir(archivo_entrada):
    print("="*70)
    print(" CONVERSOR RAW A CSV Y PRONOSTICO")
    print(f" Fecha: {datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')} UTC")
    print("="*70)
    print(f"\nProcesando archivo: {archivo_entrada}")
    
    if not os.path.exists(archivo_entrada):
        print(f"\nERROR: No se encontro el archivo '{archivo_entrada}'")
        return None
    
    try:
        with open(archivo_entrada, 'rb') as f:
            datos = f.read()
        
        print(f"Tamaño del archivo: {len(datos)} bytes")
        print("Limpiando datos binarios...")
        
        texto = datos.decode('ascii', errors='ignore')
        texto = texto.replace('\x00', '')
        texto = texto.replace('\r', '')
        texto = re.sub(r'[\x01-\x08\x0B-\x0C\x0E-\x1F\x7F-\xFF]', '', texto)
        
        lineas_brutas = texto.split('\n')
        registros = []
        
        for linea in lineas_brutas:
            linea = linea.strip()
            if not linea:
                continue
            if 'ID,Fecha,Hora' in linea:
                continue
            if es_linea_valida_csv(linea):
                registros.append(linea)
        
        if len(registros) == 0:
            print("\nADVERTENCIA: No se encontraron datos validos")
            return None
        
        registros_unicos = []
        ids_vistos = set()
        
        for registro in registros:
            id_reg = registro.split(',')[0]
            if id_reg not in ids_vistos:
                registros_unicos.append(registro)
                ids_vistos.add(id_reg)
        
        registros_unicos.sort(key=lambda x: int(x.split(',')[0]))
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        archivo_salida = f"datos_meteorologicos_{timestamp}.csv"
        
        with open(archivo_salida, 'w', encoding='utf-8') as f:
            f.write('ID,Fecha,Hora,TempC,Hum\n')
            for registro in registros_unicos:
                f.write(registro + '\n')
        
        print(f"\n{'='*70}")
        print(f"CONVERSION EXITOSA")
        print(f"{'='*70}")
        print(f"Archivo CSV generado: {archivo_salida}")
        print(f"Ubicacion: {os.path.abspath(archivo_salida)}")
        print(f"Total de registros: {len(registros_unicos)}")
        
        return archivo_salida
    
    except Exception as e:
        print(f"\nERROR: {e}")
        return None

def calcular_estadisticas(valores):
    n = len(valores)
    if n == 0:
        return None
    
    valores_ordenados = sorted(valores)
    
    count = n
    mean = sum(valores) / n
    
    varianza = sum((x - mean) ** 2 for x in valores) / n
    std = math.sqrt(varianza)
    
    minimo = min(valores)
    maximo = max(valores)
    
    q1_pos = (n - 1) * 0.25
    q1 = valores_ordenados[int(q1_pos)]
    
    q2_pos = (n - 1) * 0.50
    q2 = valores_ordenados[int(q2_pos)]
    
    q3_pos = (n - 1) * 0.75
    q3 = valores_ordenados[int(q3_pos)]
    
    return {
        'Count': count,
        'Mean': mean,
        'Std': std,
        'Min': minimo,
        '25%': q1,
        '50%': q2,
        '75%': q3,
        'Max': maximo
    }

def regresion_lineal(x_valores, y_valores):
    n = len(x_valores)
    if n < 2:
        return None, None
    
    x_mean = sum(x_valores) / n
    y_mean = sum(y_valores) / n
    
    numerador = sum((x_valores[i] - x_mean) * (y_valores[i] - y_mean) for i in range(n))
    denominador = sum((x_valores[i] - x_mean) ** 2 for i in range(n))
    
    if denominador == 0:
        return y_mean, 0
    
    pendiente = numerador / denominador
    intercepto = y_mean - pendiente * x_mean
    
    return intercepto, pendiente

def pronosticar(valores, pasos=6):
    n = len(valores)
    if n < 3:
        return [valores[-1]] * pasos if valores else [0] * pasos
    
    x_valores = list(range(n))
    y_valores = valores
    
    intercepto, pendiente = regresion_lineal(x_valores, y_valores)
    
    if intercepto is None:
        return [valores[-1]] * pasos
    
    pronosticos = []
    for i in range(1, pasos + 1):
        valor_futuro = intercepto + pendiente * (n + i - 1)
        pronosticos.append(round(valor_futuro, 1))
    
    return pronosticos

def generar_excel_estadisticas(archivo_csv):
    try:
        with open(archivo_csv, 'r', encoding='utf-8') as f:
            lineas = f.readlines()
        
        if len(lineas) <= 1:
            print("No hay datos suficientes")
            return None
        
        temps = []
        hums = []
        
        for linea in lineas[1:]:
            partes = linea.strip().split(',')
            if len(partes) >= 5:
                try:
                    temps.append(int(partes[3]))
                    hums.append(int(partes[4]))
                except:
                    pass
        
        if not temps or not hums:
            print("No se pudieron extraer datos")
            return None
        
        stats_temp = calcular_estadisticas(temps)
        stats_hum = calcular_estadisticas(hums)
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        archivo_stats = f"estadisticas_{timestamp}.csv"
        
        with open(archivo_stats, 'w', encoding='utf-8') as f:
            f.write('Estadistica,Temperatura_C,Humedad_%\n')
            f.write(f'Count,{stats_temp["Count"]},{stats_hum["Count"]}\n')
            f.write(f'Mean,{stats_temp["Mean"]:.2f},{stats_hum["Mean"]:.2f}\n')
            f.write(f'Std,{stats_temp["Std"]:.2f},{stats_hum["Std"]:.2f}\n')
            f.write(f'Min,{stats_temp["Min"]:.0f},{stats_hum["Min"]:.0f}\n')
            f.write(f'25%,{stats_temp["25%"]:.0f},{stats_hum["25%"]:.0f}\n')
            f.write(f'50%,{stats_temp["50%"]:.0f},{stats_hum["50%"]:.0f}\n')
            f.write(f'75%,{stats_temp["75%"]:.0f},{stats_hum["75%"]:.0f}\n')
            f.write(f'Max,{stats_temp["Max"]:.0f},{stats_hum["Max"]:.0f}\n')
        
        print(f"\n{'='*70}")
        print("RESUMEN ESTADISTICO")
        print(f"{'='*70}")
        print(f"Archivo generado: {archivo_stats}")
        print(f"Ubicacion: {os.path.abspath(archivo_stats)}")
        print(f"\n{'ESTADISTICA':<15} {'TEMPERATURA (C)':>15} {'HUMEDAD (%)':>15}")
        print("-" * 70)
        print(f"{'Count':<15} {stats_temp['Count']:>15} {stats_hum['Count']:>15}")
        print(f"{'Mean':<15} {stats_temp['Mean']:>15.2f} {stats_hum['Mean']:>15.2f}")
        print(f"{'Std':<15} {stats_temp['Std']:>15.2f} {stats_hum['Std']:>15.2f}")
        print(f"{'Min':<15} {stats_temp['Min']:>15.0f} {stats_hum['Min']:>15.0f}")
        print(f"{'25%':<15} {stats_temp['25%']:>15.0f} {stats_hum['25%']:>15.0f}")
        print(f"{'50%':<15} {stats_temp['50%']:>15.0f} {stats_hum['50%']:>15.0f}")
        print(f"{'75%':<15} {stats_temp['75%']:>15.0f} {stats_hum['75%']:>15.0f}")
        print(f"{'Max':<15} {stats_temp['Max']:>15.0f} {stats_hum['Max']:>15.0f}")
        print("-" * 70)
        
        return archivo_stats
        
    except Exception as e:
        print(f"Error en analisis: {e}")
        return None

def generar_excel_pronostico(archivo_csv):
    try:
        with open(archivo_csv, 'r', encoding='utf-8') as f:
            lineas = f.readlines()
        
        if len(lineas) <= 1:
            print("No hay datos suficientes")
            return None
        
        temps = []
        hums = []
        ultima_hora = None
        ultima_fecha = None
        
        for linea in lineas[1:]:
            partes = linea.strip().split(',')
            if len(partes) >= 5:
                try:
                    temps.append(int(partes[3]))
                    hums.append(int(partes[4]))
                    ultima_fecha = partes[1]
                    ultima_hora = partes[2]
                except:
                    pass
        
        if len(temps) < 3:
            print("Se necesitan al menos 3 registros para pronosticar")
            return None
        
        pronostico_temp = pronosticar(temps, pasos=6)
        pronostico_hum = pronosticar(hums, pasos=6)
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        archivo_pronostico = f"pronostico_{timestamp}.csv"
        
        try:
            hora_actual = datetime.strptime(ultima_hora, "%H:%M:%S")
        except:
            hora_actual = datetime.now()
        
        with open(archivo_pronostico, 'w', encoding='utf-8') as f:
            f.write('Paso,Hora,Temperatura_C,Humedad_%,Tendencia_Temp\n')
            
            for i in range(6):
                hora_futura = hora_actual + timedelta(hours=i+1)
                hora_str = hora_futura.strftime("%H:%M:%S")
                
                if i == 0:
                    tendencia = "Base"
                else:
                    diff = pronostico_temp[i] - pronostico_temp[i-1]
                    if diff > 0.5:
                        tendencia = "Subiendo"
                    elif diff < -0.5:
                        tendencia = "Bajando"
                    else:
                        tendencia = "Estable"
                
                f.write(f'{i+1},{hora_str},{pronostico_temp[i]:.1f},{pronostico_hum[i]:.1f},{tendencia}\n')
        
        print(f"\n{'='*70}")
        print("PRONOSTICO METEOROLOGICO - PROXIMAS 6 HORAS")
        print(f"{'='*70}")
        print(f"Archivo generado: {archivo_pronostico}")
        print(f"Ubicacion: {os.path.abspath(archivo_pronostico)}")
        print(f"Modelo: Regresion Lineal con Tendencia")
        print(f"Basado en: {len(temps)} registros historicos")
        print(f"Ultima lectura: {ultima_fecha} {ultima_hora}")
        print("-" * 70)
        
        print(f"\n{'PASO':<8} {'HORA':>12} {'TEMP (C)':>15} {'HUM (%)':>15} {'TENDENCIA':>18}")
        print("-" * 70)
        
        for i in range(6):
            hora_futura = hora_actual + timedelta(hours=i+1)
            hora_str = hora_futura.strftime("%H:%M:%S")
            
            if i == 0:
                tendencia_temp = "Base"
            else:
                diff = pronostico_temp[i] - pronostico_temp[i-1]
                if diff > 0.5:
                    tendencia_temp = "Subiendo"
                elif diff < -0.5:
                    tendencia_temp = "Bajando"
                else:
                    tendencia_temp = "Estable"
            
            print(f"{i+1:<8} {hora_str:>12} {pronostico_temp[i]:>15.1f} {pronostico_hum[i]:>15.1f} {tendencia_temp:>18}")
        
        print("-" * 70)
        
        tendencia_general = pronostico_temp[5] - temps[-1]
        if tendencia_general > 2:
            mensaje = "Tendencia: CALENTAMIENTO significativo esperado"
        elif tendencia_general > 0.5:
            mensaje = "Tendencia: Ligero aumento de temperatura"
        elif tendencia_general < -2:
            mensaje = "Tendencia: ENFRIAMIENTO significativo esperado"
        elif tendencia_general < -0.5:
            mensaje = "Tendencia: Ligera disminucion de temperatura"
        else:
            mensaje = "Tendencia: Temperatura ESTABLE"
        
        print(f"\n{mensaje}")
        print(f"Cambio esperado en 6h: {tendencia_general:+.1f}°C")
        print("="*70)
        
        return archivo_pronostico
        
    except Exception as e:
        print(f"Error en pronostico: {e}")
        import traceback
        traceback.print_exc()
        return None

def menu_principal(archivo_csv):
    archivos_generados = []
    
    while True:
        print(f"\n{'='*70}")
        print("MENU PRINCIPAL - ANALISIS METEOROLOGICO")
        print(f"{'='*70}")
        print("1. Generar resumen estadistico (CSV para Excel)")
        print("2. Generar pronostico de 6 horas (CSV para Excel)")
        print("3. Generar ambos")
        print("4. Ver archivos generados")
        print("5. Salir")
        print("="*70)
        
        opcion = input("\nSelecciona una opcion (1-5): ").strip()
        
        if opcion == '1':
            archivo = generar_excel_estadisticas(archivo_csv)
            if archivo:
                archivos_generados.append(archivo)
        elif opcion == '2':
            archivo = generar_excel_pronostico(archivo_csv)
            if archivo:
                archivos_generados.append(archivo)
        elif opcion == '3':
            archivo1 = generar_excel_estadisticas(archivo_csv)
            archivo2 = generar_excel_pronostico(archivo_csv)
            if archivo1:
                archivos_generados.append(archivo1)
            if archivo2:
                archivos_generados.append(archivo2)
        elif opcion == '4':
            print(f"\n{'='*70}")
            print("ARCHIVOS CSV GENERADOS:")
            print(f"{'='*70}")
            if archivos_generados:
                for i, archivo in enumerate(archivos_generados, 1):
                    print(f"{i}. {archivo}")
                    print(f"   {os.path.abspath(archivo)}")
            else:
                print("No se han generado archivos aun")
            print(f"{'='*70}")
        elif opcion == '5':
            print("\nSaliendo...")
            break
        else:
            print("Opcion invalida")
    
    return archivos_generados

if __name__ == "__main__":
    print("\n")
    
    if len(sys.argv) > 1:
        archivo = sys.argv[1]
    else:
        print("Opciones:")
        print("1. Arrastra datos_raw.txt aqui")
        print("2. Escribe la ruta completa")
        print("3. Presiona ENTER para usar 'datos_raw.txt'")
        print()
        archivo = input("> ").strip().strip('"')
    
    if not archivo:
        archivo = "datos_raw.txt"
        print(f"Usando: {archivo}")
    
    resultado = limpiar_y_convertir(archivo)
    
    if resultado:
        print(f"\nArchivo CSV base generado: {resultado}")
        archivos = menu_principal(resultado)
        
        print(f"\n{'='*70}")
        print(f"RESUMEN FINAL")
        print(f"{'='*70}")
        print(f"Archivos generados:")
        print(f"1. Datos originales: {resultado}")
        for archivo in archivos:
            if 'estadisticas' in archivo:
                print(f"2. Estadisticas: {archivo}")
            elif 'pronostico' in archivo:
                print(f"3. Pronostico: {archivo}")
        print(f"\nTodos listos para abrir en Excel")
        print(f"{'='*70}")
    
    input("\nPresiona ENTER para salir...")