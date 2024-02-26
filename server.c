//Mathias Costantino
//Matricola: 20043922
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

const char WELCOME_MESSAGE[] = "OK START Benvenuto, mandami i tuoi dati\n";
const char OK_DATA_MESSAGE[] = "OK DATA ";
const char OK_STATS_MESSAGE[] = "OK STATS ";
const char ERR_SYNTAX_MESSAGE[] = "ERR SYNTAX Il messaggio non è sintatticamente corretto.\n";
const char ERR_VARIANZA[] = "ERR STATS Non posso calcolare la varianza di 1 campione.\n";
const char ERR_ZEROVALORI[] = "ERR STATS Non posso calcolare media e varianza se non ho nessun valore.\n";
const char ERR_COERENTE[] = "ERR DATA Il valore dichiarato non è coerente con il contenuto del messaggio.\n";

double potenza(double base, int esponente)
{
    double risultato = 1.0;

    if (esponente > 0)
    {
        for (int i = 0; i < esponente; i++)
        {
            risultato *= base;
        }
    }
    else if (esponente < 0)
    {
        for (int i = 0; i > esponente; i--)
        {
            risultato /= base;
        }
    }

    return risultato;
}

void calcola_media_varianza(const int *dati, int num_dati, double *media, double *varianza)
{
    *media = 0.0;
    *varianza = 0.0;

    // Calcola la media
    for (int i = 0; i < num_dati; i++)
    {
        *media += dati[i];
    }
    *media /= num_dati;

    // Calcola la varianza
    for (int i = 0; i < num_dati; i++)
    {
        *varianza += potenza(dati[i] - *media, 2);
    }
    *varianza /= num_dati;
}

int main(int argc, char *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    // variabili per salvare dati caricati
    int *dati_ricevuti = NULL;
    int numerototale = 0;

    struct sockaddr_in simpleServer;

    if (2 != argc)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
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

    simplePort = atoi(argv[1]);

    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0)
    {
        fprintf(stderr, "Bind completed!\n");
    }
    else
    {
        fprintf(stderr, "Could not bind to address!\n");
        close(simpleSocket);
        exit(1);
    }

    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1)
    {
        fprintf(stderr, "Cannot listen on socket!\n");
        close(simpleSocket);
        exit(1);
    }

    while (1)
    {
        struct sockaddr_in clientName = {0};
        int simpleChildSocket = 0;
        socklen_t clientNameLength = sizeof(clientName);

        simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);

        if (simpleChildSocket == -1)
        {
            fprintf(stderr, "Cannot accept connections!\n");
            close(simpleSocket);
            exit(1);
        }

        write(simpleChildSocket, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));

        while (1)
        {
            char buffer[512];
            int bytes_received = read(simpleChildSocket, buffer, sizeof(buffer) - 1);
            if (bytes_received <= 0)
            {
                perror("Error receiving data from client");
                close(simpleChildSocket);
                break;
            }

            buffer[bytes_received] = '\0';

            // variabile che indica il numero di dati da caricare
            // primo numero che si scrive
            int num_data;

            if (sscanf(buffer, "%d", &num_data) != 1)
            {
                // Invia messaggio di errore sintattico
                write(simpleChildSocket, ERR_SYNTAX_MESSAGE, strlen(ERR_SYNTAX_MESSAGE));
                //numerototale = 0;
                //free(dati_ricevuti);
                //close(simpleSocket);
                //return 0;
                continue;
            }

            //Ho aggiunto questo per mandare messaggio di errore ERR_COERENTE
            //solo nel caso il primo numero risulti maggiore dei dati inviati
            //altrimenti non inviava ERR_COERENTE
            //ma solo nel caso num_data < dei numeri inviati
            if (num_data > 0 && num_data > (bytes_received - 2)) // -2 per considerare lo spazio e il newline dopo il primo numero
            {
              write(simpleChildSocket, ERR_COERENTE, strlen(ERR_COERENTE));
              continue;
            }

            int counter = 0;

            if (num_data > 0)
            {
                // Alloca un array per memorizzare i dati
                dati_ricevuti = (int *)realloc(dati_ricevuti, (numerototale + num_data) * sizeof(int));
                if (dati_ricevuti == NULL)
                {
                    fprintf(stderr, "Errore nell'allocazione di memoria.\n");
                    close(simpleChildSocket);
                    break;
                }

                // ciclo for per tenere traccia dei valori inseriti
                // aumento counter
                // cosi da controllare con num_data se i valori sono uguali
                // altrimenti errore e chiudo connessione
                for (int j = 0; buffer[j] != '\0' && buffer[j] != '\n'; j++)
                {
                    // Incremento il contatore solo se il carattere corrente è uno spazio
                    if (buffer[j] == ' ')
                    {
                        counter++;

                        // Ignora gli spazi bianchi consecutivi
                        // per evitare di contare uno spazio in più come valore in più
                        while (buffer[j + 1] == ' ')
                        {
                            j++;
                        }

                        //se prossimo carattere '\0' e decremento
                        if (buffer[j + 1] == '\0')
                        {
                            counter--;
                        }
                    }
                }

                // debug
                // printf("Ricevuti %d dati: ", num_data);

                char *data_str = strtok(buffer, " "); // Prima chiamata a strtok
                for (int i = 0; i < num_data; i++)
                {
                    data_str = strtok(NULL, " "); // Chiamate successive a strtok
                    if (data_str == NULL)
                    {
                        fprintf(stderr, "Errore nella lettura dei dati.\n");
                        close(simpleChildSocket);
                        break;
                    }
                    dati_ricevuti[numerototale + i] = atoi(data_str);
                    // debug
                    // printf("%d ", dati_ricevuti[numerototale + i]);
                }

                //debug
                //printf("Ricevuti %d dati: \n", num_data);
                //fflush(stdout);

                //printf("Ricevuti %d contatore: \n", counter);
                //fflush(stdout);

                if (counter != num_data)
                {
                    // Invia il messaggio di errore al client
                    write(simpleChildSocket, ERR_COERENTE, strlen(ERR_COERENTE));

                    // Libera la memoria e chiudi la connessione
                    //numerototale = 0;
                    //free(dati_ricevuti);
                    close(simpleChildSocket);
                    continue;
                    //return 0;
                }
                // Invia OK DATA con il numero totale di dati ricevuti
                char ok_data_message[256];
                sprintf(ok_data_message, "%s%d\n", OK_DATA_MESSAGE, num_data);
                write(simpleChildSocket, ok_data_message, strlen(ok_data_message));

                numerototale += num_data;

                double mean, variance;
                calcola_media_varianza(dati_ricevuti, num_data, &mean, &variance);
            }
            else
            {
                // debug
                //  Stampa l'array completo
                // printf("Array completo: ");
                // for (int i = 0; i < numerototale; i++) {
                // printf("%d ", dati_ricevuti[i]);
                //}
                // printf("\n");

                // Caso in cui il client manda 0 dati
                if (numerototale == 0)
                {
                    // se ho 0 valori non posso calcolare media e varianza
                    //  Invia messaggio di errore sintattico
                    // libero array e azzerro
                    // chiudo connessione
                    numerototale = 0;
                    free(dati_ricevuti);
                    dati_ricevuti = NULL;
                    write(simpleChildSocket, ERR_ZEROVALORI, strlen(ERR_ZEROVALORI));
                    //close(simpleSocket);
                    //return 0;
                    continue;
                }
                else if (numerototale == 1)
                {
                    // Se ho un solo valore non posso calcolare varianza
                    //  Invia messaggio di errore sintattico
                    // libero array e azzerro
                    // chiudo connessione
                    numerototale = 0;
                    free(dati_ricevuti);
                    dati_ricevuti = NULL;
                    write(simpleChildSocket, ERR_VARIANZA, strlen(ERR_VARIANZA));
                    //close(simpleSocket);
                    //return 0;
                    continue;
                }
                else
                {
                    // Invia OK STATS con il risultato
                    double mean, variance;
                    calcola_media_varianza(dati_ricevuti, numerototale, &mean, &variance);

                    char ok_stats_message[256];
                    sprintf(ok_stats_message, "%s%d %.1f %.1f\n", OK_STATS_MESSAGE, numerototale, mean, variance);
                    write(simpleChildSocket, ok_stats_message, strlen(ok_stats_message));

                    // resettare tutto dopo calcolato media e varianza
                    // cosi ripeti i calcoli senza prendere i valori precedenti
                    // svuoto array e resetto a 0 il numero totale dei valori Array
                    numerototale = 0;
                    free(dati_ricevuti);
                    dati_ricevuti = NULL;

                    // Chiudi la connessione
                    close(simpleChildSocket);
                    continue;
                    //return 0;
                }
            }
        }
    }
    close(simpleSocket);
    return 0;
}
