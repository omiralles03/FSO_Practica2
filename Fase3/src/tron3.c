#include "game.h"
#include <fcntl.h>
#include <stdio.h> /* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* programa principal				    */
int main(int n_args, const char *ll_args[]) {

    srand(getpid()); /* inicialitza numeros aleatoris */

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

    printf("Joc del Tron\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> "
           "sortir\n",
           TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
    printf("prem una tecla per continuar:\n");
    getchar();

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

    int id_retwin = ini_mem(retwin);
    char *p_retwin = map_mem(id_retwin);
    win_set(p_retwin, n_fil, n_col);

    int id_mem = ini_mem(sizeof(shared_data));
    sd = map_mem(id_mem);
    sd->fi1 = 0;
    sd->fi2 = 0;
    sd->num_opo = num_opo;
    for (int i = 0; i < n_fil; i++) {
        for (int j = 0; j < n_col; j++) {
            sd->ocupada[i][j] = 0;
        }
    }
    sem_draw = ini_sem(1);
    sem_log = ini_sem(1);
    sem_sd = ini_sem(1);

    pid_t pid_update = fork();
    if (pid_update == 0) {
        while (!sd->fi1 && !sd->fi2) {
            waitS(sem_draw);
            win_update();
            signalS(sem_draw);
            win_retard(10);
        }
        exit(0);
    }

    inicialitza_joc();

    char s_idretwin[16], s_idmem[16], s_nfil[16], s_ncol[16], s_sdraw[16],
        s_slog[16], s_ssd[16], s_varia[16], s_maxret[16], s_minret[16],
        s_logfile[16], s_index[16];

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
            // srand(getpid()); /* inicialitza numeros aleatoris */
            // mou_oponent(i);
            execlp("./oponent3", "oponent3", s_idretwin, s_idmem, s_nfil,
                   s_ncol, s_sdraw, s_slog, s_ssd, s_varia, s_maxret, s_minret,
                   s_logfile, s_index, NULL);
            exit(0);
        }
    }

    mou_usuari();

    while (wait(NULL) > 0) {
    }

    elim_sem(sem_draw);
    elim_sem(sem_log);
    elim_sem(sem_sd);

    win_fi(); /* tanca les curses */
    free(p_usu);
    free(p_opo); /* allibera la memoria dinamica obtinguda */

    if (sd->fi1 == -1)
        printf("S'ha aturat el joc amb tecla RETURN!\n\n");
    else {
        if (sd->fi2)
            printf("Ha guanyat l'usuari!\n\n");
        else
            printf("Ha guanyat l'ordinador!\n\n");
    }

    elim_mem(id_mem);
    close(log_file);

    return (0);
}
