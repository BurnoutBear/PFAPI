#include <stdio.h>
#include <stdlib.h>

// Definizione dei colori per i nodi del Red-Black Tree
#define BLACK 0
#define RED 1

// Definizione di una stazione di servizio
typedef struct Station {
    int distance;
    int num_cars;
    int *autonomy;
    int color;
    struct Station *left;
    struct Station *right;
    struct Station *parent;
} Station;

// Nodo sentinella utilizzato per rappresentare il "nil"
Station NIL_NODE = {0, 0, NULL, BLACK, NULL, NULL, NULL};
#define NIL (&NIL_NODE)

char readCommand();
int readInt();
void end_of_line();
void rb_insert(Station**, Station*);
void rb_insert_fixup(Station**, Station*);
void rb_delete(Station**, Station*);
void rb_delete_fixup(Station**, Station*);
void left_rotate(Station**, Station*);
void right_rotate(Station**, Station*);
Station *rb_search(Station*, int);
void rb_transplant(Station**, Station*, Station*);
Station *tree_minimum(Station*);
void free_tree(Station*);
void aggiungi_stazione(Station**, int, int, int*);
void demolisci_stazione(Station**, int);
void aggiungi_auto(Station*, int);
void rottama_auto(Station*, int, int);
void pianifica_percorso(Station*, int, int, int);
void getStations1(Station*, int*, int, int, int*);
void getStations2(Station*, int*, int, int, int*);
void set_steps1(const int*, const int*, int*);
void set_steps2(const int*, const int*, int*);
void find_path1(int*, const int*);
void find_path2(const int*, const int*, const int*, int*);

int main() {
    Station *root = NIL;
    int num_stations = 0;

    //Lettura dei comandi da stdin
    char c;
    c = readCommand();
    while(c != EOF) {
        if(c == 's') {
            int distance, num_cars, gas;
            distance = readInt();
            if(NIL == rb_search(root, distance)) {
                num_cars = readInt();
                int *autonomy = (int*) malloc(512 * sizeof(int));
                for(int i = 0; i < num_cars; i++) {
                    gas = readInt();
                    if(gas < 0)
                        gas = 0 - gas;
                    if(gas > autonomy[0]) {
                        autonomy[i] = autonomy[0];
                        autonomy[0] = gas;
                    }else
                        autonomy[i] = gas;
                }
                aggiungi_stazione(&root, distance, num_cars, autonomy);
                num_stations++;
            }else {
                printf("non aggiunta\n");
                end_of_line();
            }
        }else if(c == 'd') {
            int distance;
            distance = readInt();
            demolisci_stazione(&root, distance);
        }else if(c == 'a') {
            int distance, autonomy;
            distance = readInt();
            autonomy = readInt();
            Station *s = rb_search(root, distance);
            if(s != NIL)
                aggiungi_auto(s, autonomy);
            else
                printf("non aggiunta\n");
        }else if(c == 'r') {
            int distance, autonomy;
            distance = readInt();
            autonomy = readInt();
            rottama_auto(root, distance, autonomy);
        }else if(c == 'p') {
            int start_distance, end_distance;
            start_distance = readInt();
            end_distance = readInt();
            if(NIL != rb_search(root, start_distance) && NIL != rb_search(root, end_distance))
                pianifica_percorso(root, start_distance, end_distance, num_stations);
            else
                printf("nessun percorso\n");
        }

        c = readCommand();
    }

    // Deallocazione del RBT
    free_tree(root);

    return 0;
}

char readCommand() {
    char c, x;
    c = (char) getchar();

    if(c == '\n')
        c = (char) getchar();

    if(c == EOF)
        return EOF;

    if(c == 'a') {
        do
            c = (char) getchar();
        while(c != '-');
        c = (char) getchar();
    }

    do {
        x = (char) getchar();
    }while(x != ' ');

    return c;
}

int readInt() {
    int i, x = 0;

    do {
        i = getchar();
        if(i == -1)
            return (0-x);

        if(i == 32 || i == 10 || i == 13) {
            return x;
        }

        i -= 48;
        x *= 10;
        x += i;

    }while(1);
}

void end_of_line() {
    int x;
    do {
        x = getchar();
    }while(x != 10 && x != 13);
}

// Funzione per l'inserimento di un nodo nel Red-Black Tree
void rb_insert(Station **root, Station *z) {
    Station *prev = NIL;
    Station *curr = *root;


    while(curr != NIL) {
        prev = curr;
        if(z->distance < curr->distance) {
            curr = curr->left;
        }else {
            curr = curr->right;
        }
    }

    z->parent = prev;
    if (prev == NIL) {
        *root = z;
    } else if (z->distance < prev->distance) {
        prev->left = z;
    } else {
        prev->right = z;
    }

    z->left = NIL;
    z->right = NIL;
    z->color = RED;
    rb_insert_fixup(root, z);
}

// Funzione per la correzione delle proprietà del Red-Black Tree dopo l'inserimento
void rb_insert_fixup(Station **root, Station *z) {
    while(z != *root && z->parent->color == RED) {
        if(z->parent == z->parent->parent->left) {
            Station *y = z->parent->parent->right;
            if(y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            }else {
                if(z == z->parent->right) {
                    z = z->parent;
                    left_rotate(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                right_rotate(root, z->parent->parent);
            }
        }else {
            Station *y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            }else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                left_rotate(root, z->parent->parent);
            }
        }
    }

    (*root)->color = BLACK;
}

// Funzione per l'eliminazione di un nodo dal Red-Black Tree
void rb_delete(Station **root, Station *z) {
    Station *y = z;
    Station *x;
    int y_og_color = y->color;

    if(z->left == NIL) {
        x = z->right;
        rb_transplant(root, z, z->right);
    }else if(z->right == NIL) {
        x = z->left;
        rb_transplant(root, z, z->left);
    }else {
        y = tree_minimum(z->right);
        y_og_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        }else {
            rb_transplant(root, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rb_transplant(root, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if(y_og_color == BLACK) {
        rb_delete_fixup(root, x);
    }

    free(z->autonomy);
    free(z);
}

// Funzione per la correzione delle proprietà del Red-Black Tree dopo l'eliminazione
void rb_delete_fixup(Station **root, Station *x) {
    while(x != *root && x->color == BLACK) {
        if(x == x->parent->left) {
            Station *w = x->parent->right;
            if(w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                left_rotate(root, x->parent);
                w = x->parent->right;
            }
            if(w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            }else {
                if(w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    right_rotate(root, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                left_rotate(root, x->parent);
                x = *root;
            }
        }else {
            Station *w = x->parent->left;
            if(w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                right_rotate(root, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    left_rotate(root, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                right_rotate(root, x->parent);
                x = *root;
            }
        }
    }

    x->color = BLACK;
}

// Funzione per la rotazione sinistra di un nodo nel Red-Black Tree
void left_rotate(Station **root, Station *x) {
    Station *y = x->right;
    x->right = y->left;
    if (y->left != NIL) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NIL) {
        *root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

// Funzione per la rotazione destra di un nodo nel Red-Black Tree
void right_rotate(Station **root, Station *y) {
    Station *x = y->left;
    y->left = x->right;
    if (x->right != NIL) {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == NIL) {
        *root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    x->right = y;
    y->parent = x;
}

// Funzione per la ricerca di una stazione nel Red-Black Tree
Station* rb_search(Station *root, int distance) {
    Station *x = root;
    //printf("ROOT = %d, X = %d\n", root->distance, x->distance);
    while(x != NIL) {
        //printf("%d - ", x->distance);
        if(x->distance == distance) {
            return x;
        }else if(x->distance > distance) {
            x = x->left;
        }else {
            x = x->right;
        }
    }
    return NIL;
}

// Funzione per sostituire un sottoalbero con un altro sottoalbero nel Red-Black Tree
void rb_transplant(Station **root, Station *u, Station *v) {
    if (u->parent == NIL) {
        *root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}


// Restituisce il minimo nodo di un Red-Black Tree
Station* tree_minimum(Station *node) {
    while(node->left != NIL) {
        node = node->left;
    }
    return node;
}

// Funzione per liberare la memoria utilizzata dal Red-Black Tree
void free_tree(Station *node) {
    if(node != NIL) {
        free_tree(node->left);
        free_tree(node->right);
        free(node->autonomy);
        free(node);
    }
}

// Funzione per l'inserimento di una stazione nel Red-Black Tree
void aggiungi_stazione(Station **root, int distance, int num_cars, int *autonomy) {
    Station *new_station = (Station *)malloc(sizeof(Station));
    new_station->distance = distance;
    new_station->num_cars = num_cars;
    new_station->autonomy = autonomy;
    new_station->color = RED;
    new_station->left = NIL;
    new_station->right = NIL;
    new_station->parent = NIL;
    rb_insert(root, new_station);
    printf("aggiunta\n");
}

// Funzione per la demolizione di una stazione dal Red-Black Tree
void demolisci_stazione(Station **root, int distance) {
    Station *z = rb_search(*root, distance);
    if(z != NIL) {
        rb_delete(root, z);
        printf("demolita\n");
    }else {
        printf("non demolita\n");
    }
}

// Funzione per aggiungere un'auto a una stazione
void aggiungi_auto(Station *s, int autonomy) {
    //printf("num_cars: %d\nautonomy: %d\nautonomy[0]: %d\n", s->num_cars, autonomy, s->autonomy[0]);
    if(autonomy > s->autonomy[0]) {
        s->autonomy[s->num_cars] = s->autonomy[0];
        s->autonomy[0] = autonomy;
    }else
        s->autonomy[s->num_cars] = autonomy;

    s->num_cars++;
    printf("aggiunta\n");
}

// Funzione per rottamare un'auto da una stazione
void rottama_auto(Station *root, int distance, int autonomy) {
    Station *station = rb_search(root, distance);
    if(station != NIL) {
        int i, num_cars = station->num_cars;
        for(i=0; i < num_cars; i++) {
            if(station->autonomy[i] == autonomy) {
                if(i==0) {
                    station->autonomy[0] = station->autonomy[1];
                    // Shift delle automobili successive
                    for(i=2; i < num_cars; i++) {
                        if(station->autonomy[i] > station->autonomy[0]) {
                            station->autonomy[i-1] = station->autonomy[0];
                            station->autonomy[0] = station->autonomy[i];
                        }else {
                            station->autonomy[i-1] = station->autonomy[i];
                        }
                    }
                }else {
                    //Shift delle automobili successive
                    for(i++; i < num_cars; i++) {
                        station->autonomy[i-1] = station->autonomy[i];
                    }
                }

                station->num_cars--;
                printf("rottamata\n");
                return;
            }
        }
    }
    printf("non rottamata\n");
}

//Funzione di pianificazione del percorso
void pianifica_percorso(Station* root, int start, int end, int num_stations) {

    int *stations = (int *) malloc(sizeof(int) * (num_stations + 1));
    int *autonomies = (int *) malloc(sizeof(int) * (num_stations + 1));
    int *steps = (int *) malloc(sizeof(int) * (num_stations + 1));
    int *path = (int *) malloc(sizeof(int) * (num_stations + 1));
    int i;

    stations[0] = 0;
    path[0] = start;

    if(start < end) {
        getStations1(root, stations, start, end, autonomies);
        for(i = 0; i < stations[0]; i++)
            steps[i] = -1;
        steps[i] = 0;
        set_steps1(stations, autonomies, steps);
        /*
        printf("Stations:");
        for(i = 1; i <= stations[0]; i++)
            printf(" %d(%d)[%d]", stations[i], autonomies[i], steps[i]);
        printf("\n");
        */
        if(steps[1] != -1)
            find_path1(stations, steps);
        else
            printf("nessun percorso\n");
    }else {
        getStations2(root, stations, start, end, autonomies);
        for(i = 0; i < stations[0]; i++)
            steps[i] = 1;
        steps[i] = 0;
        set_steps2(stations, autonomies, steps);
        if(steps[1] != 1 && stations[0] != 2) {
            path[steps[1]] = end;
            find_path2(stations, autonomies, steps, path);
        }else if(steps[1] == 1 && stations[0] == 2){
            printf("%d %d\n", stations[1], stations[2]);
        }else
            printf("nessun percorso\n");
    }

    free(stations);
    free(autonomies);
    free(steps);
    free(path);
}

// Funzione che prende le stazioni tra start ed end e la loro autonomia massima
void getStations1(Station *root, int* stations, int start, int end, int *autonomies) {
    if(root == NIL)
        return;

    if(root->distance > start)
        getStations1(root->left, stations, start, end, autonomies);

    if(root->distance >= start && root->distance <= end) {
        stations[0]++;
        stations[stations[0]] = root->distance;
        autonomies[stations[0]] = root->autonomy[0];
    }

    if(root->distance <= end)
        getStations1(root->right, stations, start, end, autonomies);
}

//Stessa cosa della funzione prima ma quando start > end
void getStations2(Station *root, int* stations, int start, int end, int *autonomies) {
    if(root == NIL)
        return;

    if(root->distance < start)
        getStations2(root->right, stations, start, end, autonomies);

    if(root->distance >= end && root->distance <= start) {
        stations[0]++;
        stations[stations[0]] = root->distance;
        autonomies[stations[0]] = root->autonomy[0];
    }

    if(root->distance >= end)
        getStations2(root->left, stations, start, end, autonomies);
}

// Funzione che segna quanti passi ci metto da ogni stazione all'ultima
void set_steps1(const int *stations, const int *autonomies, int *steps) {
    int i = stations[0] - 1, j, range;
    for(; i > 0; i--) {
        range = stations[i] + autonomies[i];
        for(j = 1; i+j <= stations[0] && stations[i+j] <= range; j++)
            if(steps[i+j] != -1 && (steps[i] == -1 || steps[i] > steps[i+j] + 1))
                steps[i] = steps[i+j] + 1;
    }
}

//Funzione che segna quanti passi ci metto da ogni stazione all'ultima
void set_steps2(const int *stations, const int *autonomies, int *steps) {
    int i = stations[0] - 1, j, range;
    for(; i > 0; i--) {
        range = stations[i] - autonomies[i];
        for(j = 1; i+j <= stations[0] && stations[i+j] >= range; j++)
            if(steps[i+j] != 1 && (steps[i] == 1 || steps[i] < steps[i+j] - 1))
                steps[i] = steps[i+j] - 1;
    }
    /*
    printf("Stations:");
    for(i = 1; i <= stations[0]; i++)
        printf(" %d(%d)[%d]", stations[i], autonomies[i], steps[i]);
    printf("\n\n");
    */
    if(steps[1] != 1) {
        steps[1] *= -1;
        for(i = 1; i < stations[0]; i++) {
            range = stations[i] - autonomies[i];
            for(j = 1; i + j <= stations[0] && stations[i + j] >= range; ++j)
                if((0 - steps[i + j]) == steps[i] - 1)
                    steps[i + j] *= -1;
        }
        /*
        printf("\n\nStations:");
        for(i = 1; i <= stations[0]; i++)
            printf(" %d(%d)[%d]", stations[i], autonomies[i], steps[i]);
        printf("\n");   */
    }
}

void find_path1(int *stations, const int *steps){
    int i, stp;

    printf("%d", stations[1]);
    for(stp = steps[1], i = 2; i <= stations[0]; i++)
        if(steps[i] == stp-1) {
            printf(" %d", stations[i]);
            stp--;
        }
    printf("\n");
}

void find_path2(const int *stations, const int *autonomies, const int *steps, int *path) {
    int i, j, stp = 0, dist;

    while(stp != steps[1]) {
        stp = 0;
        dist = stations[stations[0]];
        for(i = stations[0]; i > 1;) {
            path[steps[1] - steps[i]] = stations[i];
            for(j = i - 1; j > 0; j--) {
                if(steps[j] == stp + 1) {
                    if(dist >= stations[j] - autonomies[j]) {
                        stp++;
                        dist = stations[j];
                        i = j;
                        break;
                    }
                }
            }
        }
    }

    printf("%d", path[0]);
    for (i = 1; i <= steps[1]; i++) {
        printf(" %d", path[i]);
    }
    printf("\n");
}
