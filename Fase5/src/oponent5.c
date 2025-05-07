#include "../libs/game.h"
#include <fcntl.h>
#include <iso646.h>
#include <pthread.h>
#include <stdio.h> /* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    int opo_index;
    int bustia;
} thread_args;

typedef struct {
    int f;
    int c;
    int index;
} paint_args;

void *paint_fwd(void *arg) {

    paint_args *p_args = (paint_args *)arg;
    int len, hit = -1;
    trail_data trails;

    if (DEBUG) {
        printf("Contingut: [%d, %d]\n", p_args->f, p_args->c);
    }
    waitS(sem_sd);
    len = sd->trails[p_args->index].len;
    trails = sd->trails[p_args->index];
    signalS(sem_sd);

    for (int i = 0; i < len; i++) {
        if (trails.trail[i].f == p_args->f && trails.trail[i].c == p_args->c) {
            hit = i;
            break;
        }
    }

    if (hit == -1) {
        free(arg);
        return NULL;
    }

    for (int i = hit; hit < len; i++) {
        int f = trails.trail[i].f;
        int c = trails.trail[i].c;

        // TODO: Check for potential locks whithout win_retard
        waitS(sem_draw);
        win_escricar(f, c, '*', INVERS);
        signalS(sem_draw);
        win_retard(10);
    }

    free(p_args);
    return NULL;
}

void *paint_bwd(void *arg) {

    paint_args *p_args = (paint_args *)arg;
    int len, hit = -1;
    trail_data trails;

    waitS(sem_sd);
    len = sd->trails[p_args->index].len;
    trails = sd->trails[p_args->index];
    signalS(sem_sd);

    for (int i = 0; i < len; i++) {
        if (trails.trail[i].f == p_args->f && trails.trail[i].c == p_args->c) {
            hit = i;
            break;
        }
    }

    if (hit == -1) {
        free(arg);
        return NULL;
    }

    for (int i = hit; i >= 0; i--) {
        int f = trails.trail[i].f;
        int c = trails.trail[i].c;

        waitS(sem_draw);
        win_escricar(f, c, '$', INVERS);
        signalS(sem_draw);
        win_retard(10);
    }

    free(p_args);
    return NULL;
}

void *msg_listener(void *arg) {
    thread_args *thargs = (thread_args *)arg;
    msg_data msg;

    while (1) {
        int res = receiveM(thargs->bustia, &msg);

        if (DEBUG) {
            printf("Missatge rebut, bytes: %d (expected: %zu)\n", res,
                   sizeof(msg_data));
            printf("Contingut: [%d, %d]\n", msg.f, msg.c);
        }

        if (res > 0) {
            paint_args *fwd_args = malloc(sizeof(paint_args));
            fwd_args->f = msg.f;
            fwd_args->c = msg.c;
            fwd_args->index = thargs->opo_index;

            paint_args *bwd_args = malloc(sizeof(paint_args));
            bwd_args->f = msg.f;
            bwd_args->c = msg.c;
            bwd_args->index = thargs->opo_index;

            pthread_t fwd, bwd;
            pthread_create(&fwd, NULL, paint_fwd, fwd_args);
            pthread_create(&bwd, NULL, paint_bwd, bwd_args);

            pthread_join(fwd, NULL);
            pthread_join(bwd, NULL);
        }
    }
    return NULL;
}

/* programa principal				    */
int main(int n_args, const char *ll_args[]) {

    if (n_args != 14) {
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
    int bustia = atoi(ll_args[13]);

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
    signalS(sem_sd);

    thread_args *args = malloc(sizeof(thread_args));
    args->opo_index = index;
    args->bustia = bustia;
    pthread_t tid;
    pthread_create(&tid, NULL, msg_listener, args);
    pthread_detach(tid);

    srand(getpid());
    mou_oponent(index);

    return (0);
}
