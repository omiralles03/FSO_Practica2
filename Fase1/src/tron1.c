#include "../libs/winsuport.h" /* incloure definicions de funcions propies */
#include <fcntl.h>
#include <stdio.h> /* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_OPO 9

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

/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o */
/* de l'oponent */
void esborrar_posicions(pos p_pos[], int n_pos) {
    int i;

    for (i = n_pos - 1; i >= 0; i--) /* de l'ultima cap a la primera */
    {
        win_escricar(p_pos[i].f, p_pos[i].c, ' ',
                     NO_INV); /* esborra una pos. */
        win_retard(10);       /* un petit retard per simular el joc real */
    }
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void) {
    char strin[45];

    usu.f = (n_fil - 1) / 2;
    usu.c = (n_col) / 4; /* fixa posicio i direccio inicial usuari */
    usu.d = 3;
    win_escricar(usu.f, usu.c, '0',
                 INVERS);   /* escriu la primer posicio usuari */
    p_usu[n_usu].f = usu.f; /* memoritza posicio inicial */
    p_usu[n_usu].c = usu.c;
    n_usu++;

    for (int i = 0; i < num_opo; i++) {
        opo[i].f = (n_fil - 1) * (i + 1) / (num_opo + 1);
        opo[i].c =
            (n_col * 3) / 4; /* fixa posicio i direccio inicial oponent */
        opo[i].d = 1;
        win_escricar(opo[i].f, opo[i].c, i + '1',
                     INVERS);      /* escriu la primer posicio oponent */
        p_opo[n_opo].f = opo[i].f; /* memoritza posicio inicial */
        p_opo[n_opo].c = opo[i].c;
        n_opo++;
    }

    sprintf(strin, "Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
            TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
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

    do {
        canvi = 0;
        seg.f = opo[index].f + df[opo[index].d]; /* calcular seguent posicio */
        seg.c = opo[index].c + dc[opo[index].d];
        cars = win_quincar(seg.f, seg.c); /* calcula caracter seguent posicio */
        if (cars != ' ')                  /* si seguent posicio ocupada */
            canvi = 1; /* anotar que s'ha de produir un canvi de direccio */
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
                fi2 = 1; /* xoc: ha perdut l'oponent! */
            else {
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
            win_escricar(opo[index].f, opo[index].c, '1' + index,
                         INVERS);          /* dibuixa bloc oponent */
            p_opo[n_opo].f = opo[index].f; /* memoritza posicio actual */
            p_opo[n_opo].c = opo[index].c;
            n_opo++;
            escriu_log(index + 1, opo[index].f, opo[index].c, opo[index].d,
                       fi2);
        } else
            esborrar_posicions(p_opo, n_opo);

        win_retard(rand() % (max_ret - min_ret + 1) + min_ret);

    } while (!fi1 && !fi2); /* repetir fins que no xoca amb res */
}

/* funcio per moure l'usuari una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si ha xocat     */
/* contra alguna cosa, i 0 altrament */
void mou_usuari(void) {
    char cars;
    tron seg;
    int tecla;

    do {
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
                fi1 = -1;
                break;
            }
        seg.f = usu.f + df[usu.d]; /* calcular seguent posicio */
        seg.c = usu.c + dc[usu.d];
        cars =
            win_quincar(seg.f, seg.c); /* calcular caracter seguent posicio */
        if (cars == ' ')               /* si seguent posicio lliure */
        {
            usu.f = seg.f;
            usu.c = seg.c;                           /* actualitza posicio */
            win_escricar(usu.f, usu.c, '0', INVERS); /* dibuixa bloc usuari */
            p_usu[n_usu].f = usu.f; /* memoritza posicio actual */
            p_usu[n_usu].c = usu.c;
            n_usu++;
            escriu_log(0, usu.f, usu.c, usu.d, fi1);
        } else {
            esborrar_posicions(p_usu, n_usu);
            fi1 = 1;
        }

        win_retard(rand() % (max_ret - min_ret + 1) + min_ret);
    } while (!fi1 && !fi2);
}

/* programa principal				    */
int main(int n_args, const char *ll_args[]) {
    int retwin; /* variables locals */

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
    retwin = win_ini(&n_fil, &n_col, '+', INVERS); /* intenta crear taulell */

    if (retwin < 0) /* si no pot crear l'entorn de joc amb les curses */
    {
        fprintf(stderr, "Error en la creacio del taulell de joc:\t");
        switch (retwin) {
        case -1:
            fprintf(stderr, "camp de joc ja creat!\n");
            break;
        case -2:
            fprintf(stderr, "no s'ha pogut inicialitzar l'entorn de curses!\n");
            break;
        case -3:
            fprintf(stderr, "les mides del camp demanades son massa grans!\n");
            break;
        case -4:
            fprintf(stderr, "no s'ha pogut crear la finestra!\n");
            break;
        }
        exit(2);
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
    inicialitza_joc();
    fi1 = 0;
    fi2 = 0;

    pid_t pid_opo[num_opo];
    for (int i = 0; i < num_opo; i++) {
        pid_opo[i] = fork();
        if (pid_opo[i] == 0) {
            srand(getpid()); /* inicialitza numeros aleatoris */
            mou_oponent(i);
            exit(0);
        }
    }

    mou_usuari();
    while (wait(NULL) > 0)
        ;

    win_fi(); /* tanca les curses */
    free(p_usu);
    free(p_opo); /* allibera la memoria dinamica obtinguda */

    if (fi1 == -1)
        printf("S'ha aturat el joc amb tecla RETURN!\n\n");
    else {
        if (fi2)
            printf("Ha guanyat l'usuari!\n\n");
        else
            printf("Ha guanyat l'ordinador!\n\n");
    }

    close(log_file);

    return (0);
}
