//Mathias Costantino
//Matricola: 20043922
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

void send_message(int socket, const char *message)
{
    send(socket, message, strlen(message), 0);
}

int receive_message(int socket)
{
    char buffer[512];
    int bytes_received = read(socket, buffer, sizeof(buffer) - 1);

    if (bytes_received <= 0)
    {
        perror("Error receiving data from server");
        close(socket);
        exit(1);
    }

    buffer[bytes_received] = '\0'; // Termina la stringa ricevuta

    // Modifica il messaggio di benvenuto
    if (strstr(buffer, "OK START") != NULL)
    {
      printf("%s\n\n", "Benvenuto nel programma di calcolo della media e varianza.");
      printf("%s", "Per terminare il programma inserisci solo lo 0 come numero.\n");
      printf("%s\n", "*) Inserisci una serie di numeri.");
      printf("%s", "*) Il primo valore da inserire rappresenta il numero di valori che si vuole inserire.\n");
      printf("%s", "*) Il resto dei numeri rappresentano i valori da calcolare.\n\n");
      printf("%s\n", "ATTENZIONE: Il primo valore inserito e il resto dei numeri caricati devono corrispondere, altrimenti sara' errore.\n");
    }
    else
    {
      // Verifica se il messaggio è "ERR STATS"
      char *err_stats_message = strstr(buffer, "ERR STATS ");
      if (err_stats_message != NULL)
      {
          //Stampo la stringa senza ERR STATS
          printf("%s\n", err_stats_message + strlen("ERR STATS "));
          return 1;
      }
      else
      {
        // Verifica se il messaggio è "OK STATS"
        char *ok_stats_message = strstr(buffer, "OK STATS");
        if (ok_stats_message != NULL)
        {
          int numdata;
          float media, varianza;
          sscanf(ok_stats_message, "OK STATS %d %f %f", &numdata, &media, &varianza);
          printf("I valori calcolati sono.\n\n*)Numero campioni:\t%d\n*)Media:\t\t%.2f\n*)Varianza:\t\t%.2f\n", numdata, media, varianza);
        }
        else{
          // Verifica se il messaggio è "ERR SYNTAX"
          char *err_syntax_message = strstr(buffer, "ERR SYNTAX ");
          if (err_syntax_message != NULL)
          {
              //Stampo stringa senza ERR SYNTAX
              printf("%s\n", err_syntax_message + strlen("ERR SYNTAX "));
              return 1;
          }
          else{
            // Verifica se il messaggio è "ERR DATA"
            char *err_data_message = strstr(buffer, "ERR DATA ");
            if (err_data_message != NULL)
            {
                //Stampo stringa senza ERR DATA
                printf("%s\n", err_data_message + strlen("ERR DATA "));
                return 1;
            }
            else{
              // Verifica se il messaggio è "OK DATA"
                    char *ok_data_message = strstr(buffer, "OK DATA ");
                    if (ok_data_message != NULL)
                    {
                        int num_data;
                        sscanf(ok_data_message, "OK DATA %d", &num_data);
                        printf("Hai inserito %d dati.\n\n", num_data);
                    }
                    else{
                          printf("S: %s\n", buffer);
                    }
                  }
                }
              }
            }
          }

    //se arriva messaggio ERR
    if (strncmp(buffer, "ERR", 3) == 0)
    {
        return 1;
    }

    //se arriva 0 come messaggio
    if (strcmp(buffer, "0") == 0)
    {
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    int errore = 0;
    char buffer[256] = "";
    struct sockaddr_in simpleServer;

    if (3 != argc)
    {
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1)
    {
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }
    else
    {
        fprintf(stderr, "Socket created!\n");
    }

    simplePort = atoi(argv[2]);

    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0)
    {
        fprintf(stderr, "Connect successful!\n\n");
    }
    else
    {
        fprintf(stderr, "Could not connect to address!\n");
        close(simpleSocket);
        exit(1);
    }

    //Messaggio di Benvenuto
    receive_message(simpleSocket);

    // Invia i dati al server
    while (1)
    {
        printf("Inserire serie di numeri: ");
        fgets(buffer, sizeof(buffer), stdin);

        // Rimuovi il carattere di nuova riga dalla stringa
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }

        //Ho aggiunto questa verifica per permettere di evitare di scrivere
        //qualche carattere insieme a qualche numero
        //perchè se si scriveva, per esempio
        //2 10 ccc
        //ccc lo considerava come dato, cosi evito di contarlo
        int all_digits = 1;
        for (size_t i = 0; i < len - 1; i++)
        {
          if (!isdigit(buffer[i])  && buffer[i] != ' ')
          {
            all_digits = 0;
            break;
          }
        }

        if (!all_digits)
        {
          fprintf(stderr, "Hai inserito qualche lettera, riscrivi i numeri grazie.\n");
          continue;
        }


        send_message(simpleSocket, buffer);

        // Ricevi e stampa la risposta del server
        errore = receive_message(simpleSocket);

        // Esci dal ciclo se il messaggio ricevuto è "ERR ..."
        if (errore==1)
        {
            break;
        }
        if (strcmp(buffer, "0") == 0) {
            break;
        }
    }

    close(simpleSocket);

    return 0;
}
