Projeto Redes de Computadores
1º Semestre, P2, 2023/2024 - LEIC@IST (Alameda)

Grupo #57 (Turno RCL08)
- Gonçalo Alves, Nº 103540
- Guilherme Belchior, Nº 102447

------------------ Introdução -------------------

Neste projeto, o objetivo foi implementar uma plataforma de leilões, em que os
clientes utilizam um programa para abrir, fechar e listar leilões de forma
a venderem os seus ativos, e a poderem criar propostas para estes leilões.

Para este efeito, o programa de cada cliente contacta um servidor que gera todos
os leilões e utilizadores que se encontram numa base de dados. A comunicação
entre os programas de cliente e o servidor são feitos através do protocolos UDP
e TCP.

---------- Como compilar este projeto -----------

Abra um terminal na diretoria onde se encontra este documento e corra o comando
"make". Este script irá compilar os programas de cliente e de servidor através
do gcc.

O programa do cliente ("user") será colocado na sub-pasta "client" (esta pasta
será criada caso não exista). Para correr este programa, entre nesta sub-diretoria
e corra o seguinte comando:

./user [-n IP_servidor] [-p porto_servidor]

Verifique se os ativos que serão usados durante a execução do programa se encontram
nesta mesma sub-diretoria.

O programa do servidor ("AS") será colocado na sub-pasta "ASDIR" (esta pasta
também vai ser criada caso não exista). Para correr este programa, entre nesta
sub-diretoria e corra o seguinte comando:

./AS [-v] [-p porto_servidor]

Ao iniciar este programa, serão criadas duas pastas ("AUCTIONS" e "USERS") na
diretoria de execução do servidor, de modo a que se possam salvaguardar os dados
dos leilões e utilizadores do sistema.

------- Explicação dos ficheiros de código ------

O código base do programa do cliente encontra-se no ficheiro "auction_client.c".
Este ficheiro trata principalmente da leitura e veificação dos comandos dados
pelo utilizador, e gerar as mensagens para serem depois enviadas.

A manipulação das mensagens com os protocolos TCP e UDP são maioritariamente
feitos no ficheiro "client_connections.c".

O código base do programa do servidor encontra-se no ficheiro "auction_server.c".
Este ficheiro aguarda por mensagens vindas dos sockets de TCP e UDP e depois
verifica os respetivos argumentos para depois fazer a manipulação dos registos
na base de dados.
Este ficheiro de código também usa as funções do ficheiro "client_connections.c"
para fazer o envio e receção das mensagens por TCP e UDP.

O ficheiro "database_handling.c" faz a ligação entre o servidor e o sistema de
ficheiros que corresponde à base de dados com os registos dos utilizadores, 
leilões e propostas.

O ficheiro "file.handling.c" contém funções úteis para a verificação da existência
de ficheiros na base de dados, e também para o envio e receção do conteúdo dos
ficheiros dos ativos, associados aos respetivos leilões.

O ficheiro "utils.c" contém funções de utilidade, como a ativação do modo verboso.

-------------------------------------------------