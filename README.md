# IAA

## Visualizador de instancias

Para ver como se van "consumiendo" las instancias de [https://github.com/elimail/Multi-depotBridgeInspectionProblem/tree/main](repositorio de Profesora Elizabeth Montero), se deben seguir los siguientes pasos

Para compilar el código para mostrar el resultado de las instancias generadas

```
g++ -std=c++17 -Wall -Wextra -O2 generate_instance.cpp instance.cpp -o <mostrar_instancia>
```

Una vez compilado para visualizar una instancia cualquiera, en su terminal inserte:

```
./<mostrar_instancia> <ruta de la instancia que se quiere visualizar>
```

Por ejemplo visualizando "<mostrar_instancia> = visualizador" y visualizar la instancia "instances/Instance_b10_a4_d3_s129.dat"

```
./visualizador instances/Instance_b10_a4_d3_s129.dat
```

hola
