#include "memoria.h"
#include "semafor.h"
#include "winsuport2.h" /* incloure definicions de funcions propies */
#include <fcntl.h>
#include <stdio.h> /* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_OPO 9
#define MAX_FIL 1000
#define MAX_COL 1000

/* definir estructures d'informacio */
typedef struct { /* per un tron (usuari o oponent) */
    int f;       /* posicio actual: fila */
    int c;       /* posicio actual: columna */
    int d;       /* direccio actual: [0..3] */
} tron;

typedef struct { /* per una entrada de la taula de posicio */
    int f;
    int c;
} pos;

/* variables globals */
int n_fil, n_col; /* dimensions del camp de joc */

tron opo[MAX_OPO]; /* informacio de l'oponent */

int df[] = {-1, 0, 1, 0}; /* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1}; /* dalt, esquerra, baix, dreta */

int varia; /* valor de variabilitat dels oponents [0..9] */

pos *p_opo;    /* els jugadors */
int n_opo = 0; /* numero d'entrades en les taules de pos. */

int fi1, fi2;
int log_file;
int max_ret, min_ret;

typedef struct {
    int num_opo;
    int fi1, fi2;
    char ocupada[MAX_FIL][MAX_COL];
} shared_data;

shared_data *sd;
int sem_draw, sem_log, sem_sd;

/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o */
/* de l'oponent */
void esborrar_posicions(pos p_pos[], int n_pos) {
    int i;

    for (i = n_pos - 1; i >= 0; i--) /* de l'ultima cap a la primera */
    {
        waitS(sem_draw);
        win_escricar(p_pos[i].f, p_pos[i].c, ' ',
                     NO_INV); /* esborra una pos. */
        signalS(sem_draw);
        win_retard(10); /* un petit retard per simular el joc real */
    }
}

/* funcio per mostrar informacio de joc a la finestra inferior */
void show_score() {

    char strin[70];
    if (n_col < 100)
        sprintf(strin, "Num. oponents: %d", sd->num_opo);
    else
        sprintf(
            strin,
            "Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir \t Num. "
            "oponents: %d\n",
            TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER, sd->num_opo);

    win_escristr(strin);
}

/* funcio per escriure al log */
void escriu_log(int id, int f, int c, int d, int fi) {
    char msg[60]; // Increased buffer size for safety
    const char *dir[4] = {"UP", "LEFT", "DOWN", "RIGHT"};

    if (id == 0)
        sprintf(msg, "USER   { pos: [%3d,%3d] | dir: %-5s | fi: %d }\n", f, c,
                dir[d], fi);
    else
        sprintf(msg, "OPO %-2d { pos: [%3d,%3d] | dir: %-5s | fi: %d }\n", id,
                f, c, dir[d], fi);

    write(log_file, msg, strlen(msg));
}

/* funcio per moure un oponent una posicio; retorna 1 si l'oponent xoca */
/* contra alguna cosa, 0 altrament					*/
void mou_oponent(int index) {
    char cars;
    tron seg;
    int k, vk, nd, vd[3];
    int canvi = 0;
    int local_fi1;

    do {
        waitS(sem_sd);
        local_fi1 = sd->fi1;
        signalS(sem_sd);
        if (local_fi1 != 0)
            break; /* si l'usuari ha acabat el joc, sortir */

        canvi = 0;
        seg.f = opo[index].f + df[opo[index].d]; /* calcular seguent posicio */
        seg.c = opo[index].c + dc[opo[index].d];

        waitS(sem_sd);
        if (sd->ocupada[seg.f][seg.c]) {
            cars = 'X';
        } else {
            cars = win_quincar(seg.f,
                               seg.c); /* calcula caracter seguent posicio */
        }
        signalS(sem_sd);

        if (cars != ' ') /* si seguent posicio ocupada */
            canvi = 1;   /* anotar que s'ha de produir un canvi de direccio */
        else if (varia > 0) /* si hi ha variabilitat */
        {
            k = rand() % 10; /* prova un numero aleatori del 0 al 9 */
            if (k < varia)
                canvi = 1; /* possible canvi de direccio */
        }

        if (canvi) /* si s'ha de canviar de direccio */
        {
            nd = 0;
            for (k = -1; k <= 1; k++) /* provar direccio actual i dir. veines */
            {
                vk = (opo[index].d + k) % 4; /* nova direccio */
                if (vk < 0)
                    vk += 4; /* corregeix negatius */
                seg.f =
                    opo[index].f + df[vk]; /* calcular posicio en la nova dir.*/
                seg.c = opo[index].c + dc[vk];

                waitS(sem_sd);
                if (sd->ocupada[seg.f][seg.c]) {
                    cars = 'X';
                } else {
                    cars = win_quincar(
                        seg.f, seg.c); /* calcula caracter seguent posicio */
                }
                signalS(sem_sd);

                if (cars == ' ') {
                    vd[nd] = vk; /* memoritza com a direccio possible */
                    nd++;        /* anota una direccio possible mes */
                }
            }
            if (nd == 0) /* si no pot continuar, */
            {
                fi2 = 1; /* xoc: ha perdut l'oponent! */
                waitS(sem_sd);
                sd->num_opo--;
                if (sd->num_opo == 0) {
                    sd->fi2 = 1;
                }
                show_score();
                signalS(sem_sd);
            } else {
                if (nd == 1)              /* si nomes pot en una direccio */
                    opo[index].d = vd[0]; /* li assigna aquesta */
                else                      /* altrament */
                    opo[index].d =
                        vd[rand() % nd]; /* segueix una dir. aleatoria */
            }
        }
        if (fi2 == 0) /* si no ha col.lisionat amb res */
        {
            opo[index].f =
                opo[index].f + df[opo[index].d]; /* actualitza posicio */
            opo[index].c = opo[index].c + dc[opo[index].d];

            // Actualitzar posicio ocupada
            waitS(sem_sd);
            sd->ocupada[opo[index].f][opo[index].c] = 1;
            signalS(sem_sd);

            waitS(sem_draw);
            win_escricar(opo[index].f, opo[index].c, '1' + index,
                         INVERS); /* dibuixa bloc oponent */
            signalS(sem_draw);

            p_opo[n_opo].f = opo[index].f; /* memoritza posicio actual */
            p_opo[n_opo].c = opo[index].c;
            n_opo++;

            waitS(sem_log);
            escriu_log(index + 1, opo[index].f, opo[index].c, opo[index].d,
                       fi2);
            signalS(sem_log);
        } else
            esborrar_posicions(p_opo, n_opo);

        win_retard(rand() % (max_ret - min_ret + 1) + min_ret);

    } while (!fi2); /* repetir fins que no xoca amb res */
}

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
