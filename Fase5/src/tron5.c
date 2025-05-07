#include "../libs/game.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h> /* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void parse_args(int n_args, const char *ll_args[]) {

    num_opo = atoi(ll_args[1]);

    varia = atoi(ll_args[2]); /* obtenir parametre de variabilitat */
    if (varia < 0)
        varia = 0; /* verificar limits */
    if (varia > 3)
        varia = 3;

    log_file = open(ll_args[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    max_ret = 1000;
    min_ret = 10;
    if (n_args == 6) {
        max_ret = atoi(ll_args[4]);
        min_ret = atoi(ll_args[5]);
        if (max_ret > 1000)
            max_ret = 1000;
        if (min_ret < 10)
            min_ret = 10;
        if (max_ret < min_ret) {
            int aux_ret = max_ret;
            max_ret = min_ret;
            max_ret = aux_ret;
        }
    }

    /* Mostra les tecles de joc */
    printf("Enable DEBUG MODE? [y/N]: ");
    char res = getchar();
    DEBUG = (res == 'y' || res == 'Y') ? 1 : 0;

    printf("Joc del Tron\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> "
           "sortir\n",
           TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
    printf("prem una tecla per continuar:\n");
    getchar();
}

void game_updater(time_t timer_start) {
    time_t timer_now, timer_diff;
    int ops, secs, mins;

    pid_t pid_update = fork();

    if (pid_update == 0) {
        while (!sd->fi1 && !sd->fi2) {
            waitS(sem_draw);

            timer_now = time(NULL);
            timer_diff = (int)difftime(timer_now, timer_start);
            mins = timer_diff / 60;
            secs = timer_diff % 60;

            waitS(sem_sd);
            ops = sd->num_opo;
            signalS(sem_sd);

            char status[64];
            snprintf(status, sizeof(status),
                     "Temps: %02d:%02d  Oponents restants: %d", mins, secs,
                     ops);
            win_escristr(status);

            win_update();

            signalS(sem_draw);
            win_retard(10);
        }
        exit(0);
    }
}

void fork_oponents(int id_retwin, int id_mem) {

    char s_idretwin[16], s_idmem[16], s_nfil[16], s_ncol[16], s_sdraw[16],
        s_slog[16], s_ssd[16], s_varia[16], s_maxret[16], s_minret[16],
        s_logfile[16], s_index[16], s_bustia[16];

    snprintf(s_idretwin, sizeof(s_idretwin), "%d", id_retwin);
    snprintf(s_idmem, sizeof(s_idmem), "%d", id_mem);
    snprintf(s_nfil, sizeof(s_nfil), "%d", n_fil);
    snprintf(s_ncol, sizeof(s_ncol), "%d", n_col);
    snprintf(s_sdraw, sizeof(s_sdraw), "%d", sem_draw);
    snprintf(s_slog, sizeof(s_slog), "%d", sem_log);
    snprintf(s_ssd, sizeof(s_ssd), "%d", sem_sd);
    snprintf(s_varia, sizeof(s_varia), "%d", varia);
    snprintf(s_maxret, sizeof(s_maxret), "%d", max_ret);
    snprintf(s_minret, sizeof(s_minret), "%d", min_ret);
    snprintf(s_logfile, sizeof(s_logfile), "%d", log_file);

    pid_t pid_opo[num_opo];
    for (int i = 0; i < num_opo; i++) {
        pid_opo[i] = fork();
        if (pid_opo[i] == 0) {
            snprintf(s_index, sizeof(s_index), "%d", i);
            snprintf(s_bustia, sizeof(s_bustia), "%d", bustia[i]);

            execlp("./oponent5", "oponent5", s_idretwin, s_idmem, s_nfil,
                   s_ncol, s_sdraw, s_slog, s_ssd, s_varia, s_maxret, s_minret,
                   s_logfile, s_index, s_bustia, NULL);
            exit(0);
        }
    }
}

void cleanup(int id_retwin, int id_mem) {

    /* elimina semafors i memoria compartida */
    elim_sem(sem_draw);
    elim_sem(sem_log);
    elim_sem(sem_sd);

    elim_mem(id_mem);
    elim_mem(id_retwin);
    close(log_file);

    win_fi(); /* tanca les curses */
    free(p_usu);
    free(p_opo); /* allibera la memoria dinamica obtinguda */
}

/* programa principal				    */
int main(int n_args, const char *ll_args[]) {

    srand(getpid()); /* inicialitza numeros aleatoris */

    parse_args(n_args, ll_args); /* obtenir parametres de joc */

    n_fil = 0;
    n_col = 0; /* demanarem dimensions de taulell maximes */
    size_t retwin =
        win_ini(&n_fil, &n_col, '+', INVERS); /* intenta crear taulell */

    if (retwin < 0) /* si no pot crear l'entorn de joc amb les curses */
    {
        fprintf(stderr, "Error en la creacio del taulell de joc:\t");
    }

    p_usu =
        calloc(n_fil * n_col / 2, sizeof(pos)); /* demana memoria dinamica */
    p_opo =
        calloc(n_fil * n_col / 2, sizeof(pos)); /* per a les posicions ant. */
    if (!p_usu || !p_opo) /* si no hi ha prou memoria per als vectors de pos. */
    {
        win_fi(); /* tanca les curses */
        if (p_usu)
            free(p_usu);
        if (p_opo)
            free(p_opo); /* allibera el que hagi pogut obtenir */
        fprintf(stderr, "Error en alocatacion de memoria dinamica.\n");
        exit(3);
    }

    /* Fins aqui tot ha anat be! */

    /* inicialitzacio de pantalla compartida */
    int id_retwin = ini_mem(retwin);
    char *p_retwin = map_mem(id_retwin);
    win_set(p_retwin, n_fil, n_col);

    /* inicialitzacio de memoria compartida */
    int id_mem = ini_mem(sizeof(shared_data));
    sd = map_mem(id_mem);
    sd->fi1 = 0;
    sd->fi2 = 0;
    sd->num_opo = num_opo;
    for (int i = 0; i < num_opo; i++) { /* inicialitzacio de len */
        sd->trails[i].len = 0;
    }

    /* inicialitzacio de semafors */
    sem_draw = ini_sem(1);
    sem_log = ini_sem(1);
    sem_sd = ini_sem(1);

    /* inicialitzacio de la bustia */
    for (int i = 0; i < num_opo; i++) {
        bustia[i] = ini_mis();
        // printf("bustia[%d] = %d\n", i, bustia[i]);
    }

    /* actualitzador de pantalla */
    time_t timer_start = time(NULL);
    time_t timer_now, timer_diff;
    int secs, mins;
    game_updater(timer_start);

    inicialitza_joc();

    fork_oponents(id_retwin, id_mem); /* crea els processos oponents */

    /* crea el thread de l'usuari */
    pthread_t tid;
    if (pthread_create(&tid, NULL, mou_usuari, NULL) != 0) {
        fprintf(stderr, "No s'ha pogut crear el thread de mou_usuari\n");
        exit(1);
    }

    pthread_join(tid, NULL); /* espera que acabi el thread */

    while (wait(NULL) > 0) {
    }

    cleanup(id_retwin, id_mem); /* allibera els recursos */

    if (sd->fi1 == -1)
        printf("S'ha aturat el joc amb tecla RETURN!\n\n");
    else {
        if (sd->fi2)
            printf("Ha guanyat l'usuari!\n\n");
        else
            printf("Ha guanyat l'ordinador!\n\n");
    }

    /* escriu el temps de partida */
    timer_now = time(NULL);
    timer_diff = (int)difftime(timer_now, timer_start);
    mins = timer_diff / 60;
    secs = timer_diff % 60;
    printf("Temps de partida: %02d:%02d\n\n", mins, secs);

    return (0);
}
