/*
 * But:
 * Entrées :
 * Sortie :
 * ULB-ID:
 * paug0002
 * jche0027
 * rrab0007
*/


// --------------------- Implementation --------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>


// --------------------- Global variables --------------------------------

#define READ 0
#define WRITE 1

struct SharedDistance { // Distance minimale trouvee entre l'image à comparer et les images de la banque d'images
    int minDistance;
    char minFileName[256]; // Nom de l'image la plus similaire
};

int shmem_fd;
struct SharedDistance* sharedMemory;
size_t numberOfProcess;

volatile sig_atomic_t child1_ready = 0;
volatile sig_atomic_t child2_ready = 0;



// -------------------------------- Fonctions --------------------------------

void initSharedMemory(void) {
    const int protection = PROT_READ | PROT_WRITE;  // Lecture et écriture permises
    const int visibility = MAP_SHARED;              // Memoire partagee visible par les autres processus fils
    // Cree une zone de memoire partagee :
    shm_unlink("/my_shared_memory");  // Supprime la memoire partagee si elle existe deja
    shmem_fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);  // 0666 = accessible READ/WRITE pour tout users
    if (shmem_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);}
    if (ftruncate(shmem_fd, sizeof(struct SharedDistance)) == -1) { // Definit la taille de la memoire partagee
        perror("ftruncate");                                        // à la taille de SharedDistance
        exit(EXIT_FAILURE);                                         // (evite de perdre de la memoire)
    }   // Cree une zone de memoire mappée :
    sharedMemory = (struct SharedDistance*)mmap(NULL, sizeof(struct SharedDistance), protection, visibility, shmem_fd, 0);
    if (sharedMemory == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);}
}


void handler(int signum) {
    if (signum == SIGPIPE){
        printf("All reading ends of a pipe are closed\n");
        exit(0);
    }
    else if (signum == SIGINT){
        // Attendre la fin des processus fils
        int status;
        while(numberOfProcess > 0){
            wait(&status);
            if(WIFEXITED(status)){
                numberOfProcess --;
            }
        }
        // Suppression des mémoires mappée / partagée
        if (munmap(sharedMemory,  sizeof(struct SharedDistance)) == -1){
            perror("Mapped memory deallocation error");
            exit(EXIT_FAILURE);
        }
        if (shm_unlink("/my_shared_memory") == -1){
            perror("Shared memory deallocation error");
            exit(EXIT_FAILURE);
        }
        printf("L'utilsateur a appuyé sur CTRL+C\n");
        exit(0);
    }
    else if (signum == SIGUSR2){
        child1_ready = 1;
        child2_ready = 1;
    }
}

void cleanupSharedMemory(void) {
    if (munmap(sharedMemory,  sizeof(struct SharedDistance)) == -1){
        perror("Mapped memory deallocation error");
        exit(EXIT_FAILURE);
    }
    if (shm_unlink("/my_shared_memory") == -1) {
        perror("Shared memory deallocation error");
        exit(EXIT_FAILURE);
    }
}

// -------------------------------- Corps du code --------------------------------

int main(int argc, char* argv[]) {
    if (!(argc >= 2 && argc <= 3)){
        fprintf(stderr, "Usage: %s <image_to_compare>\n", argv[0]);
        return 1;
    }
    const char* fileToCompare = argv[1];

    // Initialise la mémoire + la distance minimale partagee à INT_MAX
    initSharedMemory();
    sharedMemory->minDistance = INT_MAX;

    // Handler personnalisé qui gere les signaux SIGPIPE et SIGINT
    signal(SIGINT, handler);
    signal(SIGPIPE, handler);
    // Pour faire commencer les processus fils en même temps
    signal(SIGUSR2, handler);

    pid_t parentPID;
    parentPID = getpid();

    pid_t child1, child2;  // Initialise 2 processus fils
    int pipe1[2], pipe2[2]; // Cree 2 pipes pour comnmuniquer avec ses 2 fils

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        return 1;
    }
    child1 = fork();
    if (child1 == 0) {
        close(pipe1[WRITE]); // Ferme le côté d'écriture du pipe
        close(pipe2[READ]);  // Ferme le côté de lecture du pipe2 (inutilisé par le fils1)
        close(pipe2[WRITE]); // Ferme le côté d'écriture du pipe2 (inutilisé par le fils1)
        while (!child1_ready) {
            sleep(1); // Peut-être remplacé par des appels système plus sophistiqués
        }
        // Code pour le premier fils
        char fileName[256];
        size_t nbytes = sizeof(fileName);
        bool finished = false;
        while (!finished){
            ssize_t nbytes_read = read(pipe1[READ], fileName, nbytes);
            if (nbytes_read < 0) {
                perror("read");
                return 1;
            }
            else if (nbytes_read == 0) {
                finished = true; // Fin de la lecture
            }
            else { 
                fileName[nbytes_read] = '\0';
                char *cheminFileName = malloc(strlen("img/") + strlen(fileName) + 1);
                snprintf(cheminFileName, strlen("img/") + strlen(fileName) + 1, "img/%s", fileName);
                if (cheminFileName != NULL) {
                    // Utilise cheminFileName ici
                    // Appel à img-dist, mise à jour de la mémoire partagée, gestion des signaux
                    // Execution de img-dist avec fileName et le chemin de l'image à comparer
                    char command[256];
                    snprintf(command, sizeof(command), "./img-dist/img-dist %s %s", fileToCompare, cheminFileName);
                    int ret = system(command)/256;
                    if (ret == -1) {
                        perror("system");
                        exit(EXIT_FAILURE);
                    }
                    int distancePHash = ret;
                    // Met à jour la distance minimale partagée si besoin
                    if (distancePHash < sharedMemory->minDistance) {
                        sharedMemory->minDistance = distancePHash;
                        strcpy(sharedMemory->minFileName, fileName);
                    }
                    free(cheminFileName);  // Libère la mémoire après utilisation
                }
            }
        }
        close(pipe1[READ]); // Ferme le côté de lecture du pipe
    }
    else if (child1 < 0) {
        perror("child1");
        return 1;
    }
    else {
        child2 = fork();
        if (child2 == 0) {
            close(pipe2[WRITE]); // Ferme le côté d'écriture du pipe
            close(pipe1[READ]);  // Ferme le côté de lecture du pipe1 (inutilisé par le fils2)
            close(pipe1[WRITE]); // Ferme le côté d'écriture du pipe1 (inutilisé par le fils2)
            while (!child2_ready) {
                sleep(1); // Peut-être remplacé par des appels système plus sophistiqués
            }
            // Code pour le deuxième fils
            char fileName[256];
            size_t nbytes = sizeof(fileName);
            bool finished = false;
            while (!finished) {
                ssize_t nbytes_read = read(pipe2[READ], fileName, nbytes);
                if (nbytes_read < 0) {
                    perror("read");
                    return 1;
                }
                else if (nbytes_read == 0) {
                    finished = true; // Fin de la lecture
                }
                else {
                    fileName[nbytes_read] = '\0'; // Ajoute le caractère de fin de chaîne
                    char *cheminFileName = malloc(strlen("img/") + strlen(fileName) + 1);
                    snprintf(cheminFileName, strlen("img/") + strlen(fileName) + 1, "img/%s", fileName);
                    if (cheminFileName != NULL) {
                        // Utilise cheminFileName ici
                        // Appel à img-dist, mise à jour de la mémoire partagée, gestion des signaux
                        // Execution de img-dist avec fileName et le chemin de l'image à comparer
                        char command[256];
                        snprintf(command, sizeof(command), "./img-dist/img-dist %s %s", fileToCompare, cheminFileName);
                        int ret = system(command)/256;
                        if (ret == -1) {
                            perror("system");
                            exit(EXIT_FAILURE);
                        }
                        int distancePHash = ret;
                        // Met à jour la distance minimale partagée si besoin
                        if (distancePHash < sharedMemory->minDistance) {
                            sharedMemory->minDistance = distancePHash;
                            strcpy(sharedMemory->minFileName, fileName);
                        }
                        free(cheminFileName);  // Libère la mémoire après utilisation
                    }
                }
            }
            close(pipe2[READ]); // Ferme le côté de lecture du pipe
        }
        else if (child2 < 0){
            perror("child2");
            return 1;
        }
        else {
            // Code pour le processus père
            close(pipe1[READ]); // Ferme le côté de lecture du premier pipe
            close(pipe2[READ]); // Ferme le côté de lecture du deuxième pipe

            // Lit depuis list-file.sh
            // Doit aussi faire sur sdtin mais avec le launcher
            int fileCountChild1 = 0; // Compteur pour le premier fils
            int fileCountChild2 = 0; // Compteur pour le deuxième fils
            char fileName[256];
            FILE* listFileOutput = popen("./list-file.sh img", "r");
            while (fgets(fileName, sizeof(fileName), listFileOutput) != NULL) {  // Lit la sortie du processus list-file.bash ligne par ligne et la stocke dans fileName tant qu'il y a des lignes à lire
                size_t len = strlen(fileName);
                if (fileName[len - 1] == '\n') {  // Supprime le saut de ligne
                    fileName[len - 1] = '\0';
                }
                // if (strcmp(fileName, fileNameToCompare) == 0){  // Pour ne pas comparer l'image à elle-même (et donc eviter une distance de 0)
                //     continue;
                // }
                // Distribue equitablement les fichiers aux 2 processus fils
                if (fileCountChild1 <= fileCountChild2) {
                    ssize_t bytes_written = write(pipe1[WRITE], fileName, sizeof(fileName));
                    if (bytes_written < 0) {
                        perror("write");
                    } else if (bytes_written == 0) {
                        break; // Fin de l'écriture
                    } else {
                        fileCountChild1++;
                    }
                } else {
                    ssize_t bytes_written = write(pipe2[WRITE], fileName, sizeof(fileName));
                    if (bytes_written < 0) {
                        perror("write");
                    } else if (bytes_written == 0) {
                        break; // Fin de l'écriture
                    } else {
                        fileCountChild2++;
                    }
                }
            }
            pclose(listFileOutput);
            close(pipe1[WRITE]); // Ferme le côté d'écriture du premier pipe
            close(pipe2[WRITE]); // Ferme le côté d'écriture du deuxième pipe

            // Envoie un signal SIGUSR2 aux 2 processus fils pour les laisser commencer à travailler
            kill(child1, SIGUSR2);
            kill(child2, SIGUSR2);

            // Attendre la fin des processus fils
            int status1, status2;
            waitpid(child1, &status1, 0);
            waitpid(child2, &status2, 0);

            // Affiche le résultat
            if (sharedMemory->minDistance == INT_MAX) {
                printf("No similar image found (no comparison could be performed successfully).\n");
            } else {
                printf("Most similar image found: '%s' with a distance of %d.\n", sharedMemory->minFileName, sharedMemory->minDistance);
            }
        }
    }

    // Supprime la memoire partagee par le processus parent uniquement
    if (getpid() == parentPID) {
        cleanupSharedMemory();
    }
    return 0;
}
