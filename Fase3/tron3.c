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

tron usu;          /* informacio de l'usuari */
tron opo[MAX_OPO]; /* informacio de l'oponent */

int df[] = {-1, 0, 1, 0}; /* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1}; /* dalt, esquerra, baix, dreta */

int varia;  /* valor de variabilitat dels oponents [0..9] */
int retard; /* valor del retard de moviment, en mil.lisegons */

pos *p_usu;               /* taula de posicions que van recorrent */
pos *p_opo;               /* els jugadors */
int n_usu = 0, n_opo = 0; /* numero d'entrades en les taules de pos. */

int fi1, fi2;
int num_opo;
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

        waitS(sem_sd);
        sd->ocupada[p_pos[i].f][p_pos[i].c] = 0; /* marcar posicio lliure */
        signalS(sem_sd);

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

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void) {
    // char strin[45];

    usu.f = (n_fil - 1) / 2;
    usu.c = (n_col) / 4; /* fixa posicio i direccio inicial usuari */
    usu.d = 3;
    win_escricar(usu.f, usu.c, '0',
                 INVERS);   /* escriu la primer posicio usuari */
    p_usu[n_usu].f = usu.f; /* memoritza posicio inicial */
    p_usu[n_usu].c = usu.c;
    n_usu++;

    sd->ocupada[usu.f][usu.c] = 1; /* Guardar posicio ocupada */

    // for (int i = 0; i < num_opo; i++) {
    //     opo[i].f = (n_fil - 1) * (i + 1) / (num_opo + 1);
    //     opo[i].c =
    //         (n_col * 3) / 4; /* fixa posicio i direccio inicial oponent */
    //     opo[i].d = 1;
    //     win_escricar(opo[i].f, opo[i].c, i + '1',
    //                  INVERS);      /* escriu la primer posicio oponent */
    //     p_opo[n_opo].f = opo[i].f; /* memoritza posicio inicial */
    //     p_opo[n_opo].c = opo[i].c;
    //     n_opo++;
    //
    //     sd->ocupada[opo[i].f][opo[i].c] = 1; /* Guardar posicio ocupada */
    // }

    show_score();
    // sprintf(strin, "Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN->
    // sortir\n",
    //         TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
    // win_escristr(strin);
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

/* funcio per moure l'usuari una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si ha xocat     */
/* contra alguna cosa, i 0 altrament */
void mou_usuari(void) {
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

        waitS(sem_sd);
        if (sd->ocupada[seg.f][seg.c]) {
            cars = 'X';
        } else {
            cars = win_quincar(seg.f,
                               seg.c); /* calcula caracter seguent posicio */
        }
        signalS(sem_sd);

        if (cars == ' ') /* si seguent posicio lliure */
        {
            usu.f = seg.f;
            usu.c = seg.c; /* actualitza posicio */

            waitS(sem_sd);
            sd->ocupada[usu.f][usu.c] = 1; /* Guardar posicio ocupada */
            signalS(sem_sd);

            waitS(sem_draw);
            win_escricar(usu.f, usu.c, '0', INVERS); /* dibuixa bloc usuari */
            signalS(sem_draw);

            p_usu[n_usu].f = usu.f; /* memoritza posicio actual */
            p_usu[n_usu].c = usu.c;
            n_usu++;

            waitS(sem_log);
            escriu_log(0, usu.f, usu.c, usu.d, fi1);
            signalS(sem_log);
        } else {
            esborrar_posicions(p_usu, n_usu);
            waitS(sem_sd);
            sd->fi1 = 1;
            signalS(sem_sd);
            fi1 = 1;
        }

        win_retard(rand() % (max_ret - min_ret + 1) + min_ret);
    } while (!fi1 && !local_fi2);
}

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
