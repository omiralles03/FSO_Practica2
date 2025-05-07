#include "game.h"
#include <fcntl.h>
#include <stdio.h> /* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* variables globals */
int n_fil, n_col; /* dimensions del camp de joc */

tron usu;          /* informacio de l'usuari */
tron opo[MAX_OPO]; /* informacio de l'oponent */

int df[4] = {-1, 0, 1, 0}; /* moviments de les 4 direccions possibles */
int dc[4] = {0, -1, 0, 1}; /* dalt, esquerra, baix, dreta */

int varia;  /* valor de variabilitat dels oponents [0..9] */
int retard; /* valor del retard de moviment, en mil.lisegons */

pos *p_usu;               /* taula de posicions que van recorrent */
pos *p_opo;               /* els jugadors */
int n_usu = 0, n_opo = 0; /* numero d'entrades en les taules de pos. */

int fi1, fi2;
int num_opo;
int log_file;
int max_ret, min_ret;

shared_data *sd;
int sem_draw, sem_log, sem_sd;

/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o
 * l'oponent*/
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

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void) {

    usu.f = (n_fil - 1) / 2;
    usu.c = (n_col) / 4; /* fixa posicio i direccio inicial usuari */
    usu.d = 3;
    win_escricar(usu.f, usu.c, '0',
                 INVERS);   /* escriu la primer posicio usuari */
    p_usu[n_usu].f = usu.f; /* memoritza posicio inicial */
    p_usu[n_usu].c = usu.c;
    n_usu++;
}

/* funcio per escriure al log */
void escriu_log(int id, int f, int c, int d, int fi) {
    char msg[60];
    const char *dir[4] = {"UP", "LEFT", "DOWN", "RIGHT"};

    if (id == 0)
        sprintf(msg, "USER   { pos: [%3d,%3d] | dir: %-5s | fi: %d }\n", f, c,
                dir[d], fi);
    else
        sprintf(msg, "OPO %-2d { pos: [%3d,%3d] | dir: %-5s | fi: %d }\n", id,
                f, c, dir[d], fi);

    write(log_file, msg, strlen(msg));
}

void *mou_usuari(void *arg) {
    char cars;
    tron seg;
    int tecla;
    int local_fi2;

    do {
        waitS(sem_sd);
        local_fi2 = sd->fi2;
        signalS(sem_sd);
        if (local_fi2 != 0)
            break; /* si tots els oponents xoquen sortir */

        tecla = win_gettec();
        if (tecla != 0)
            switch (tecla) /* modificar direccio usuari segons tecla */
            {
            case TEC_AMUNT:
                usu.d = 0;
                break;
            case TEC_ESQUER:
                usu.d = 1;
                break;
            case TEC_AVALL:
                usu.d = 2;
                break;
            case TEC_DRETA:
                usu.d = 3;
                break;
            case TEC_RETURN:
                waitS(sem_sd);
                sd->fi1 = -1; /* marcar fi del joc */
                signalS(sem_sd);
                fi1 = -1;
                break;
            }
        seg.f = usu.f + df[usu.d]; /* calcular seguent posicio */
        seg.c = usu.c + dc[usu.d];

        cars = win_quincar(seg.f, seg.c); /* calcula caracter seguent posicio */

        if (cars == ' ') /* si seguent posicio lliure */
        {
            usu.f = seg.f;
            usu.c = seg.c; /* actualitza posicio */

            waitS(sem_draw);
            win_escricar(usu.f, usu.c, '0', INVERS); /* dibuixa bloc usuari */
            signalS(sem_draw);

            p_usu[n_usu].f = usu.f; /* memoritza posicio actual */
            p_usu[n_usu].c = usu.c;
            n_usu++;

            waitS(sem_log);
            escriu_log(0, usu.f, usu.c, usu.d, fi1);
            signalS(sem_log);

        } else if (cars == '+') {
            esborrar_posicions(p_usu, n_usu);
            waitS(sem_sd);
            sd->fi1 = 1;
            signalS(sem_sd);
            fi1 = 1;
        } else {
            // TODO: ENVIAR MISSATGES
            int opo_id = cars - '1';
        }

        win_retard(rand() % (max_ret - min_ret + 1) + min_ret);
    } while (!fi1 && !local_fi2);
    return NULL;
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

        cars = win_quincar(seg.f, seg.c); /* calcula caracter seguent posicio */

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

                cars = win_quincar(
                    seg.f, seg.c); /* calcula caracter seguent posicio */

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

            waitS(sem_draw);
            win_escricar(opo[index].f, opo[index].c, '1' + index,
                         INVERS); /* dibuixa bloc oponent */
            signalS(sem_draw);

            p_opo[n_opo].f = opo[index].f; /* memoritza posicio actual */
            p_opo[n_opo].c = opo[index].c;
            n_opo++;

            /* guardar la posicio a la taula de trails */
            // TODO: REVISAR EL GUARDAT DELS TRAILS
            signalS(sem_sd);
            int l = sd->trails[index].len;
            sd->trails[index].trail[l] = p_opo[n_opo];
            waitS(sem_sd);

            waitS(sem_log);
            escriu_log(index + 1, opo[index].f, opo[index].c, opo[index].d,
                       fi2);
            signalS(sem_log);
        } else {
            esborrar_posicions(p_opo, n_opo);

            waitS(sem_sd);
            if (sd->num_opo == 0) {
                sd->fi2 = 1;
            }
            signalS(sem_sd);
        }

        win_retard(rand() % (max_ret - min_ret + 1) + min_ret);

    } while (!fi2); /* repetir fins que no xoca amb res */
}
