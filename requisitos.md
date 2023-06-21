# FTP

## Lista de Comandos

### Comandos de Sesión

- USER
- PASS
- QUIT

### Comandos de files

- PWD: present working directory
- LIST: enlista los contenidos del PWD
- CWD: change working directory
- CDUP: cd ..
- MKD: crear directorio
- RMD: eliminar directorio
- RNFR: rename from, selecciona un archivo para cambiar el nombre
- RNTO: rename to, inmediatamente después de RNFR
- DELE: elimina un archivo o directorio por nombre
- RETR: descarga un archivo al cliente
- STOR: escribe un archivo del cliente al servidor

## Procedimiento de RETR

1. Client -> solicitud RECV -> Server
2. Server -> respuesta 150 -> Client
3. Server -> data -> Client
4. Server -> respuesta 250 -> Client

## Procedimiento de STOR

1. Client -> solicitud STOR -> Server
2. Server -> respuesta 150 -> Client
3. Client -> data -> Server
4. Server -> respuesta 266 -> Client

## Servidor

- Necesita información de conexión, mínimamente
  - Estado de autenticación
  - Present working directory
  - Comandos parciales
    - USER/PASS
    - RNFR/RNTO
- Debería poder servir a más de un cliente en simultáneo (fork)

## Cliente

- Acceso a su propio files
