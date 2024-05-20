<h1 align="center">
  MonitoreoSensores  
</h1>
<h4 align="center">
  Desarrollado por:
</h4>
<table align="center">
  <tr>
    <td align="center">Lucas Rivera</td>
    <td align="center">Jose Jaramillo</td>
  </tr>
  <tr>
    <td align="center">
      <img src="https://github.com/JoseJ1709/MonitoreoSensores/assets/107308404/3dc3cdc9-4128-4e7e-8eb5-52f681db3a06" width="220" height="360">
    </td>
    <td align="center">
      <img src="https://github.com/Intro-CompuMovil/JaveWheels/assets/107308404/6a07817b-70ac-42b7-9f43-2470070f8b10" width="300">
    </td>
  </tr>
</table>
Este proyecto simula la medición de dos parámetros de una reserva de agua, específicamente el pH y la temperatura, utilizando sensores. Los datos medidos se envían a un proceso monitor que los almacena y notifica al usuario si se generan alertas.
El contenido del repositorio incluye:

* ****
* **Presentación:**
* * https://drive.google.com/file/d/1TuRbmvjJxRhwRXjoK0RaLd5vsbKJipG_/view?usp=sharing
  * En esta presentación se dará a conocer un sistema completo para la simulación y monitoreo de datos de sensores. La presentación se enfocará en los aspectos principales del sistema, incluyendo su funcionamiento, configuración y aplicaciones. Se mostrarán ejemplos de cómo utilizar el sistema para simular y monitorear datos de sensores de pH y temperatura.

* ****

* **Codigo:** Los archivos que se encuentran en el repositorio son:

* * **buffer.cpp-buffer.h** Módulos que implementan los búferes para almacenar temporalmente las medidas de los sensores.
  *  **datos.txt** Archivo de datos para pruebas.
  *   **file-ph.txt-file-temp.txt** Archivos de salida donde se almacenarán las medidas de pH y temperatura respectivamente.
  *   **monitor.cpp**Implementación del proceso monitor que gestiona los hilos recolector, H-ph y H-temperatura.
  *   **sensor.cpp** Implementación de los procesos simuladores de sensores que envían datos al monitor.
  *   **makefile**  Script de automatización para compilar y ejecutar el proyecto.
* ****

<h2 align="center">
  Ejecución
</h2>

## Compilación
En la terminal, diríjase al directorio del proyecto y ejecute el comando:
```bash
make
```

## Ejecución del Monitor
Desde el shell, invoque los procesos de los sensores de la siguiente manera:
```bash
./monitor –b tam_buffer –t file-temp –h file-ph -p pipe-nominal
```

#### Donde
- 'tam-buffer' es el tamaño de los búferes donde se colocará las medidas.
- 'file_temp' es el nombre del archivo de texto donde el hilo de temperatura colocará las
mediciones de temperatura recibidas.
- 'file_ph' es el nombre del archivo de texto donde el hilo de ph colocará las
mediciones de ph recibidas.
- 'pipe-nominal' es el pipe nominal utilizado para la comunicación con el sensor.


## Ejecución de los Sensores
Desde el shell, invoque los procesos de los sensores de la siguiente manera:
```bash
./sensor -s tipo-sensor -t tiempo -f archivo -p pipe-nominal
```

#### Donde
- 'tipo-sensor' puede ser PH o temperatura. 1 o 2.
- 'tiempo' es el intervalo de tiempo entre las medidas.
- 'archivo' es el archivo de configuración para el sensor.
- 'pipe-nominal' es el pipe nominal utilizado para la comunicación con el monitor.

<h2 align="center">
  Ejemplo
</h2>

```bash
make
```

```bash
./monitor -b 10 -t file-temp.txt -h file-ph.txt -p pipe1
```

> [!NOTE]  
>  En menos de 10 seg se debe ejecutar el sensor.

```bash
./sensor -s 2 -t 3 -f datos.txt -p pipe1
```

<h2 align="center">
  Documentacion del Desarrollo:
  <h4 align="center">
    https://github.com/JoseJ1709/MonitoreoSensores/files/15069570/Proyecto.pdf
  </h4>
</h2>

