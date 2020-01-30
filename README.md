# TFTP-Client
Cliente TFTP escrito en C. Proyecto desarrollado con fines de aprendizaje en la asignatura de Arquitectura de Redes y Servicios de Ingeniería de Software en la Universidad de Valladolid.

## Content

- [Descripción del programa](/TFTP_Description.pdf) - Requisitos para desarrollar el cliente TFTP.
- [tftp-Munumer-Blazquez.c](tftp-Munumer-Blazquez.c) - Implementación cliente TFTP.

## Development

### Requirements

- [GCC](https://gcc.gnu.org), the GNU Compiler Collection.
- TFTP server running ([RFC 1350](https://tools.ietf.org/html/rfc1350)).

### Installation

```bash
# Clone repository.
git clone https://github.com/Sergio-MB/TFTP-Client.git
cd TFTP-Client
```

### Compilation

```bash
gcc -Wall -o client.out tftp-Munumer-Blazquez.c
```

### Execution

- `-r`: read file mode.
- `-w`: write file mode.
- `-v`: tracing mode.

```bash
./client.out server-ip {-r|-w} file [-v]
```

## Deployment

### Remote Linux Virtual Machine via SSH

This C program should be executed in a provided Slackware Linux machine, so one option to send the source files to there is:

```bash
# Transfer file to remote machine
scp -P <port-number> tftp-Munumer-Blazquez.c youruser@your.machine.address:/destination/folder
```

Then, you can access to your remote machine via SSH and execute the TFTP client from there
