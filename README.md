# IAA

## Visualizador de instancias

Para ver como se van "consumiendo" las instancias de [repositorio de Profesora Elizabeth Montero](https://github.com/elimail/Multi-depotBridgeInspectionProblem/tree/main), se deben seguir los siguientes pasos

## Compilación

Todo se compila con `make` y los ejecutables quedan en `build/`:

```
make        # build/optimize, build/mostrar_instancia y build/MDIIP
make clean  # borra build/
```

`build/MDIIP` solo se compila si existe la carpeta `MDIIP/` (está en `.gitignore`); si no está, se omite.

Para visualizar una instancia cualquiera, en su terminal inserte:

```
./build/mostrar_instancia <ruta de la instancia que se quiere visualizar>
```

Por ejemplo, para visualizar la instancia "instances/Instance_b10_a4_d3_s129.dat"

```
./build/mostrar_instancia instances/Instance_b10_a4_d3_s129.dat
```

## Ejecución del optimizador

Una vez compilado con `make` se deberá ejecutar

```
./build/optimize <ruta_instancia.dat> <alpha> <epsilon> <max_iteraciones> <seed> <greedy>
```

Donde el parametro **alpha**, es que tanto porcentaje de la solución se rompe entre cada iteración, **epsilon** es la probabilidad de explorar entre cada iteración (acá se ve si se trabaja con el mejor movimiento hasta el momento o con uno no tan utilizado), **max_iteraciones** es la cantidad máxima de iteraciones **seed** es la semilla que se utiliza para los procesos aleatorios del algoritmo y **greedy** elige el constructor de la solución inicial: `P` usa el greedy propio (determinista) y `E` usa el greedy aleatorio de MDIIP (con la misma semilla genera la misma solución inicial que MDIIP). Como ejemplo de ejecución, seria

```
./build/optimize instances/Instance_b10_a4_d3_s129.dat 0.3 0.2 100 2 P
```
