# FTP

## Lista de Comandos

### Comandos de Sesión

- USER
- PASS
- QUIT

### Comandos de Filesystem

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

## Servidor

- Necesita información de conexión, mínimamente
  - Estado de autenticación
  - Present working directory
  - Comandos parciales
    - USER/PASS
    - RNFR/RNTO
- Debería poder servir a más de un cliente en simultáneo (fork)

## Cliente

- Acceso a su propio filesystem
