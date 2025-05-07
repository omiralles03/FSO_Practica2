#ifndef GAME_H
#define GAME_H

#include "memoria.h"
#include "missatge.h"
#include "semafor.h"
#include "winsuport2.h"

#define MAX_OPO 9
#define MAX_FIL 1000
#define MAX_COL 1000
#define MAX_TRAIL (MAX_FIL * MAX_COL / 2)

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

typedef struct {
    int len;              /* numero de posicions recorregudes */
    pos trail[MAX_TRAIL]; /* posicions recorregudes */
} trail_data;

typedef struct {
    int num_opo;
    int fi1, fi2;
    trail_data trails[MAX_OPO]; /* array que guarda per a cada oponent les
                                   posicions recorregudes */
} shared_data;

typedef struct {
    int f;
    int c;
} msg_data;

/* variables globals */
extern int n_fil, n_col; /* dimensions del camp de joc */

extern tron usu;          /* informacio de l'usuari */
extern tron opo[MAX_OPO]; /* informacio de l'oponent */

extern int df[4]; /* moviments de les 4 direccions possibles */
extern int dc[4]; /* dalt, esquerra, baix, dreta */

extern int varia;  /* valor de variabilitat dels oponents [0..9] */
extern int retard; /* valor del retard de moviment, en mil.lisegons */

extern pos *p_usu;       /* taula de posicions que van recorrent */
extern pos *p_opo;       /* els jugadors */
extern int n_usu, n_opo; /* numero d'entrades en les taules de pos. */

extern int fi1, fi2;
extern int num_opo;
extern int log_file;
extern int max_ret, min_ret;

extern shared_data *sd;
extern int sem_draw, sem_log, sem_sd;

extern int bustia[MAX_OPO];

extern char DEBUG;

void inicialitza_joc(void);
void escriu_log(int id, int f, int c, int d, int fi);
void esborrar_posicions(pos p_pos[], int n_pos);
void *mou_usuari(void *);
void mou_oponent(int index);

#endif
