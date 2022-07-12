# DASHBOARD ARCHITECTURE

## INTRODUCCIÓN
En el presente documento se detallan los pasos a seguir para montar en el entorno local la arquitectura que se utilizará para la visualización de datos de los sensores biomédicos

## PREREQUISITOS
- [`Git`](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git) instalado
- Entorno [`Docker`](https://docs.docker.com/desktop/) instalado

## PASOS INICIALES
1. Clonar el repositorio **Biomedical-Kit** con **HTTP** 

## OPERATIVA DE DOCKER-COMPOSE 
Para poder levantar nuestro docker-compose.yml debemos situarnos en la carpeta en la que se encuetre este archivo, en este caso la carpeta **Software**, desde esta carpeta abrir el cmd para:

### Levantar/Apagar Contenedores
- #### Levantar
	Esto iniciará los contenedores necesarios para poner en funcionamiento nuestra arquitectura Software.
	`docker-compose up`

- #### Apagar
	Esto apagará los contenedores de nuestra arquitectura Software.
    Si teniamos en el cmd ejecutandose los contenedores deberemos hacer CTRL+C para terminar los procesos y una vez terminen todos ejecutar:
	`docker-compose down`

### Kibana
Una vez que el docker-compose este levantado tendremos que acceder a la siguiente URL **http://localhost:5601/**
