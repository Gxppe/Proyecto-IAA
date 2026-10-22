# IAA

## Visualizador de instancias

Para ver como se van "consumiendo" las instancias de [repositorio de Profesora Elizabeth Montero](https://github.com/elimail/Multi-depotBridgeInspectionProblem/tree/main), se deben seguir los siguientes pasos

Para compilar el código para mostrar el resultado de las instancias generadas

```
g++ -std=c++17 -Wall -Wextra -O2 generate_instance.cpp instance.cpp -o <mostrar_instancia>
```

Una vez compilado para visualizar una instancia cualquiera, en su terminal inserte:

```
./<mostrar_instancia> <ruta de la instancia que se quiere visualizar>
```

Por ejemplo visualizando "<mostrar_instancia> = visualizador" y queriendo visualizar la instancia "instances/Instance_b10_a4_d3_s129.dat"

```
./visualizador instances/Instance_b10_a4_d3_s129.dat
```

## Ejecución del optimizador

El modo de uso para compilar la el optimizador será de la siguiente forma

```
g++ -std=c++17 -Wall -Wextra -O2 optimize.cpp optimize_utils.cpp constraints.cpp handlers.cpp instance.cpp -o optimize
```

Una vez ya compilado se deberá ejecutar

```
./optimize <ruta_instancia.dat> <alpha> <epsilon> <max_iteraciones> <seed>
```

Donde el parametro **alpha**, es que tanto porcentaje de la solución se rompe entre cada iteración, **epsilon** es la probabilidad de explorar entre cada iteración (acá se ve si se trabaja con el mejor movimiento hasta el momento o con uno no tan utilizado), **max_iteraciones** es la cantidad máxima de iteraciones y **seed** es la semilla que se utiliza para los procesos aleatorios del algoritmo. Como ejemplo de ejecución, seria

```
./optimize instances/Instance_b10_a4_d3_s129.dat 0.3 0.2 100 2
```
