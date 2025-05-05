#include "game.h"
#include <fcntl.h>
#include <stdio.h> /* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* programa principal				    */
int main(int n_args, const char *ll_args[]) {

    if (n_args != 13) {
        fprintf(stderr, "Error en els parametres");
        exit(1);
    }

    int id_retwin = atoi(ll_args[1]);
    int id_mem = atoi(ll_args[2]);
    n_fil = atoi(ll_args[3]);
    n_col = atoi(ll_args[4]);
    sem_draw = atoi(ll_args[5]);
    sem_log = atoi(ll_args[6]);
    sem_sd = atoi(ll_args[7]);
    varia = atoi(ll_args[8]);
    max_ret = atoi(ll_args[9]);
    min_ret = atoi(ll_args[10]);
    log_file = atoi(ll_args[11]);
    int index = atoi(ll_args[12]);

    char *p_retwin = map_mem(id_retwin);
    win_set(p_retwin, n_fil, n_col);

    sd = map_mem(id_mem);

    p_opo =
        calloc(n_fil * n_col / 2, sizeof(pos)); /* per a les posicions ant. */

    fi2 = 0;
    n_opo = 0;

    waitS(sem_sd);
    opo[index].f = (n_fil - 1) * (index + 1) / (sd->num_opo + 1);
    opo[index].c = (n_col * 3) / 4;
    opo[index].d = 1;
    sd->ocupada[opo[index].f][opo[index].c] = 1; /* Guardar posicio ocupada */
    signalS(sem_sd);

    srand(getpid());
    mou_oponent(index);

    return (0);
}
