#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gamelib.h"

/*-------------------------------
 |   VARIABILI GLOBALI (STATIC) |
 --------------------------------*/
static struct Zona_mondoreale* prima_zona_mondoreale = NULL;
static struct Zona_soprasotto* prima_zona_soprasotto = NULL;
static struct Giocatore* giocatori[4]; // Array di puntatori
static int numero_giocatori = 0;
static int mappa_chiusa = 0;
static char vincitori[3][50];
static int n_vincitori = 0;
/*-------------------------
 |   PROTOTIPI FUNZIONI  |
 -------------------------*/
static void genera_mappa();
static void inserisci_zona();
static void cancella_zona();
static void stampa_mappa();
static void chiudi_mappa();
static void pulizia_completa_mappa();


static enum Tipo_zona random_tipo() {
    return (enum Tipo_zona)(rand() % 10);
}

static enum Tipo_oggetto random_oggetto() {
    if ((rand() % 100) < 60)
      return nessun_oggetto;
    return (enum Tipo_oggetto)(rand() % 4 + 1);
}

static enum Tipo_nemico random_nemico(int soprasotto_attivo) {
    int val = rand() % 100;
    if (val < 50)
      return nessun_nemico;
    if (val < 80)
      return democane;
    return soprasotto_attivo ? nessun_nemico : billi;
}

static const char* nome_zona(enum Tipo_zona z) {
    switch(z) {
        case bosco:
        return "Bosco";
        case scuola:
        return "Scuola";
        case laboratorio:
        return "Laboratorio";
        case caverna:
        return "Caverna";
        case strada:
        return "Strada";
        case giardino:
        return "Giardino";
        case supermercato:
        return "Supermercato";
        case centrale_elettrica:
        return "Centrale Elett.";
        case deposito_abbandonato:
        return "Deposito Abb.";
        case stazione_polizia:
        return "Stazione Polizia";
        default:
        return "???";
    }
}

static const char* nome_nemico(enum Tipo_nemico n) {
    switch(n) {
        case nessun_nemico:
        return "Nessuno";
        case billi:
        return "Billi";
        case democane:
        return "Democane";
        case demotorzone:
        return "DEMOTORZONE";
        default: return "?";
    }
}

static const char* nome_oggetto(enum Tipo_oggetto o) {
    switch(o) {
        case nessun_oggetto:
        return "Vuoto";
        case bicicletta:
        return "Bicicletta";
        case maglietta_fuocoinferno:
        return "Maglietta Hellfire";
        case bussola:
        return "Bussola";
        case schitarrata_metallica:
        return "Chitarra Elettrica";
        default:
        return "?";
    }
}

/*---------------------------
 |   GESTIONE DELLA MEMORIA |
 ----------------------------*/
static void pulizia_completa_mappa() {
    struct Zona_mondoreale* corr_r = prima_zona_mondoreale;
    struct Zona_soprasotto* corr_s = prima_zona_soprasotto;

    while (corr_r != NULL) {
        struct Zona_mondoreale* temp_r = corr_r;
        struct Zona_soprasotto* temp_s = corr_s;

        corr_r = corr_r->avanti;
        if (corr_s) corr_s = corr_s->avanti;

        free(temp_r);
        free(temp_s);
    }
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;
}

/*-------------------------
 |   LOGICA DEL GIOCO     |
 -------------------------*/
static int controllo_nemico(struct Giocatore* g) {
    enum Tipo_nemico n = (g->mondo == 0) ? g->pos_mondoreale->nemico : g->pos_soprasotto->nemico;
    if (n != nessun_nemico) {
        printf("Nemico presente (%s)! Devi combattere.\n", nome_nemico(n));
        return 1;
    }
    return 0;
}

static void raccogli_oggetto(struct Giocatore* g) {
    // Controllo se c'è un nemico (Regola PDF: non puoi raccogliere se c'è il mostro)
    if (controllo_nemico(g))
    return;

    // Gli oggetti sono solo nel mondo reale
    if (g->mondo == 1) {
        puts("Qui nel Soprasotto non c'e' nulla di utile...");
        return;
    }

    if (g->pos_mondoreale->oggetto == nessun_oggetto) {
        puts("Non c'e' niente da raccogliere qui.");
        return;
    }

    // Cerco spazio nello zaino
    int i;
    int trovato_spazio = 0;
    for (i = 0; i < 3; i++) {
        if (g->zaino[i] == nessun_oggetto) {
            // Trovato posto vuoto
            g->zaino[i] = g->pos_mondoreale->oggetto;
            printf("Hai raccolto: %s!\n", nome_oggetto(g->zaino[i]));

            // Tolgo l'oggetto dalla stanza
            g->pos_mondoreale->oggetto = nessun_oggetto;
            trovato_spazio = 1;
            break;
        }
    }

    if (!trovato_spazio) {
        puts("Zaino pieno! Devi usare qualcosa prima.");
    }
}

static void usa_oggetto(struct Giocatore* g) {
    puts("--- ZAINO ---");
    int i;
    int vuoto = 1;
    for (i = 0; i < 3; i++) {
        printf("%d) %s\n", i+1, nome_oggetto(g->zaino[i]));
        if (g->zaino[i] != nessun_oggetto) vuoto = 0;
    }

    if (vuoto)
    {
        puts("Lo zaino e' vuoto.");
        return;
    }

    int scelta;
    printf("Quale oggetto usare? (1-3, 0 per annullare): ");
    if (scanf("%d", &scelta) != 1)
    {
      while(getchar()!='\n');
      return;
    }

    if (scelta < 1 || scelta > 3)
    return;

    // Indice array è scelta - 1
    int ind = scelta - 1;
    enum Tipo_oggetto ogg = g->zaino[ind];

    if (ogg == nessun_oggetto) {
        puts("Lì non c'è niente.");
        return;
    }

    // EFFETTI DEGLI OGGETTI
    printf("Usi %s... ", nome_oggetto(ogg));
    switch(ogg) {
        case bicicletta:
            puts("Recuperi 10 HP riposandoti.");
            g->difesa_pischica += 10;
            break;
        case maglietta_fuocoinferno:
            puts("Attacco aumentato di 5!");
            g->attacco_pischico += 5;
            break;
        case bussola:
            puts("Trovi la strada migliore. Fortuna aumentata di 5!");
            g->fortuna += 5;
            break;
        case schitarrata_metallica:
            puts("Il nemico ha subito una forte onda d'urto");
            // Se c'è un nemico, muore istantaneamente
            if (g->mondo == 0 && g->pos_mondoreale->nemico != nessun_nemico) {
                puts("Il nemico esplode !");
                g->pos_mondoreale->nemico = nessun_nemico;
            } else if (g->mondo == 1 && g->pos_soprasotto->nemico != nessun_nemico) {
                puts("Il mostro scappa via !");
                g->pos_soprasotto->nemico = nessun_nemico;
            } else {
                puts("Nessun nemico");
                g->difesa_pischica += 5;
            }
            break;
        default:
            puts("Non succede nulla");
    }

    // Rimuovo oggetto dopo l'uso
    g->zaino[ind] = nessun_oggetto;
}

static void avanza(struct Giocatore* g, int* ha_avanzato) {
    if (*ha_avanzato)
    {
      puts("Gia' mosso");
      return;
    }
    if (controllo_nemico(g))
    return;

    if (g->mondo == 0)
    {
        if (!g->pos_mondoreale->avanti)
          puts("Fine mappa");
        else
        {
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            printf("%s avanza.\n", g->nome);
            *ha_avanzato = 1;
        }
    } else {
        if (!g->pos_soprasotto->avanti)
          puts("Strada bloccata");
        else
        {
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            printf("%s avanza nel buio.\n", g->nome);
            *ha_avanzato = 1;
        }
    }
}

static void indietreggia(struct Giocatore* g)
{
    if (controllo_nemico(g))
      return;
    if (g->mondo == 1)
    {
      puts("Impossibile tornare indietro nel Soprasotto.");
      return;
    }

    if (!g->pos_mondoreale->indietro)
        puts("Sei all'inizio");
    else
    {
        g->pos_mondoreale = g->pos_mondoreale->indietro;
        g->pos_soprasotto = g->pos_soprasotto->indietro;
        printf("%s torna indietro\n", g->nome);
    }
}

static void cambia_mondo(struct Giocatore* g, int* ha_avanzato)
{
    if (g->mondo == 0)
    {
        if (*ha_avanzato)
        {
          puts("Gia' mosso");
          return;
        }
        if (controllo_nemico(g))
          return;
        g->mondo = 1;
        puts("Sei entrato nel SOPRASOTTO");
        *ha_avanzato = 1;
    } else {
        if ((rand() % 20 + 1) < g->fortuna)
        {
            g->mondo = 0;
            puts("Fuga riuscita! Sei nel mondo reale");
        } else {
            puts("Fuga fallita. Resti qui");
        }
    }
}

// Restituisce: 0=continua, 1=giocatore morto, 2=vittoria finale
static int combatti(struct Giocatore* g)
{
    enum Tipo_nemico n = (g->mondo == 0) ? g->pos_mondoreale->nemico : g->pos_soprasotto->nemico;

    if (n == nessun_nemico)
    {
      puts("Nessuno da combattere.");
      return 0;
    }
    int combattimento_attivo = 1;
    while(combattimento_attivo)
    {
      printf("----COMBATTIMENTO contro %s ----\n",nome_nemico(n));
      printf("HP: %d\n", g->difesa_pischica);
      printf("(1) Atacca\n(2)Usa Oggetto\n(3)Tenta la fuga\nScelta");

      int scelta_c;
      if (scanf("%d", &scelta_c) != 1)
      {
            while (getchar() != '\n');
            continue;
      }
        switch(scelta_c) {
          case 1:{
            int nem_atk = rand() % 15 + 10;
            int mio_atk = (rand() % 20 + 1) + g->attacco_pischico;

            printf("Tu: %d vs Nemico: %d\n", mio_atk, nem_atk);

            if (mio_atk >= nem_atk)
            {
                printf("Vittoria contro %s!\n", nome_nemico(n));

                if (n == demotorzone)
                  return 2; // Codice Vittoria

                if(rand() % 100 < 50)
                {
                  if(g->mondo == 0)
                    g->pos_mondoreale->nemico = nessun_nemico;
                  else
                    g->pos_soprasotto->nemico = nessun_nemico;
                  puts("Il nemico è stato abbattuto");
                }else
                {
                  puts("Hai vinto il round ma il nemico è ancora presente");
                }
                return 0;
                } else {
                puts("Colpito! (-5 HP)");
                g->difesa_pischica -= 5;
                if (g->difesa_pischica <= 0)
                  return 1; // Codice Morte
                }
        break;
    }
    case 2:
      usa_oggetto(g);
      break;
    case 3:
    if ((rand() % 20 + 1) < g->fortuna) // Fuga basata sulla fortuna
    {
                    puts("Sei riuscito a fuggire dallo scontro");
                    return 0;
                } else {
                    puts("Fuga fallita! Il nemico ti blocca!");
                    g->difesa_pischica -= 2; // Penalità per fuga fallita (opzionale)
                    if (g->difesa_pischica <= 0) return 1;
                }
                break;
            default:
                puts("Scelta non valida.");
        }
    }
    return 0;

}
static void registra_vincitore(char* nome)
{
    // Sposto il 2° posto al 3°
    strcpy(vincitori[2], vincitori[1]);
    // Sposto il 1° posto al 2°
    strcpy(vincitori[1], vincitori[0]);
    // Metto il nuovo vincitore al 1° posto
    strcpy(vincitori[0], nome);

    if (n_vincitori < 3) n_vincitori++;
}

/*-------------------------
 |   FUNZIONI PRINCIPALI  |
 -------------------------*/
void imposta_gioco() {
    // Inizializzo il random qui per sicurezza
    srand(time(NULL));

    // Pulisco eventuali giocatori precedenti
    int i;
    for(i=0; i<4; i++)
    {
        if (giocatori[i])
        {
          free(giocatori[i]);
          giocatori[i] = NULL;
        }
    }

    do {
        printf("Giocatori (1-4): ");
        if (scanf("%d", &numero_giocatori) != 1)
        {
        while(getchar() != '\n');//Pulisce se scrivo lettere
        continue;
        }
    } while(numero_giocatori < 1 || numero_giocatori > 4);

    while(getchar() != '\n');

    int undici_preso = 0;

    for(i=0; i<numero_giocatori; i++)
    {
        giocatori[i] = (struct Giocatore*)malloc(sizeof(struct Giocatore));
        if (!giocatori[i])
        {
          puts("Errore memoria");
          exit(1);
        } // Malloc check

        giocatori[i]->mondo = 0;
        giocatori[i]->attacco_pischico = rand() % 20 + 1;
        giocatori[i]->difesa_pischica = rand() % 20 + 1;
        giocatori[i]->fortuna = rand() % 20 + 1;

        int j;
        for(j=0; j<3; j++)
        giocatori[i]->zaino[j] = nessun_oggetto;

        printf("Nome G%d: ", i+1);
        scanf("%49s", giocatori[i]->nome); // LIMITO A 49 CHAR PER SICUREZZA
        while(getchar() != '\n');

        int sc = 0;
        do {
            printf("Vuoi modificare le statistiche?\n 1= Mantieni cosi\n 2= +3 Attacco -3 Difesa\n 3= +3 Difesa -3 Attacco\n 4= Diventa UndiciVirgolaCinque\n: ");
            if(scanf("%d", &sc) != 1){
              while(getchar() != '\n');
              sc = 0;
              continue;
            }
            if(sc==4) {
                if(undici_preso)
                {
                  puts("Occupato!");
                  sc=0;
                }
                else {
                    // Controllo lunghezza prima di concatenare
                    if (strlen(giocatori[i]->nome) < 40)
                      strcat(giocatori[i]->nome, " Undici");
                    giocatori[i]->attacco_pischico += 4;
                    giocatori[i]->difesa_pischica += 4;
                    giocatori[i]->fortuna -= 7;
                    undici_preso = 1;
                }
            } else if (sc==2) {
                 giocatori[i]->attacco_pischico += 3; giocatori[i]->difesa_pischica -= 3;
            } else if (sc==3) {
                 giocatori[i]->attacco_pischico -= 3; giocatori[i]->difesa_pischica += 3;
            }
        } while(sc < 1 || sc > 4);
    }

    mappa_chiusa = 0;
    int cmd;
    do {
        printf("\n 1)Genera la mappa\n 2)Inserisci zona\n 3)Cancella zona\n 4)Stampa la mappa\n 5)Chiudi e inizia\n Selezione: ");
        if (scanf("%d", &cmd) != 1)
        {
          while (getchar() != '\n');
          cmd = 0;
          continue;
        }
        switch(cmd) {
            case 1:
            genera_mappa();
            break;
            case 2:
            inserisci_zona();
            break;
            case 3:
            cancella_zona();
            break;
            case 4:
            stampa_mappa();
            break;
            case 5:
            chiudi_mappa();
            break;
            default:
            puts("Errato.");
        }
    } while(!mappa_chiusa);
}

void gioca() {
    if(!mappa_chiusa)
    {
      puts("Mappa non pronta.");
      return;
    }

    int i;
    for(i=0; i<numero_giocatori; i++) {
        giocatori[i]->pos_mondoreale = prima_zona_mondoreale;
        giocatori[i]->pos_soprasotto = prima_zona_soprasotto;
        giocatori[i]->mondo = 0;
    }

    int vittoria = 0;
    int tutti_morti = 0;
    int ordine[4] = {0,1,2,3}; //Array per la gestione turni

    while(!vittoria && !tutti_morti)
    {
      for (int i = numero_giocatori - 1; i > 0; i--) {
          int j = rand() % (i + 1);
          int temp = ordine[i];
          ordine[i] = ordine[j];
          ordine[j] = temp;
      }

      int conta_vivi = 0;

        for(i=0; i<numero_giocatori; i++)
        {
            int ind = ordine[i];
            if (giocatori[ind] == NULL)
            continue; // Salta i morti (che sono NULL)
            conta_vivi++;
            struct Giocatore* g = giocatori[ind];

            printf("\n--- %s ---\nHP: %d | Zona: %s (%s)\n", g->nome, g->difesa_pischica,
                   (g->mondo==0) ? nome_zona(g->pos_mondoreale->tipo) : nome_zona(g->pos_soprasotto->tipo),
                   (g->mondo==0) ? "Reale" : "Soprasotto");
            // Mostro se c'è un oggetto nella stanza (solo reale)
            if (g->mondo == 0 && g->pos_mondoreale->oggetto != nessun_oggetto)
            {
              printf("A terra vedi: %s\n", nome_oggetto(g->pos_mondoreale->oggetto));
            }
            int fine_turno = 0;
            int ha_mosso = 0;

            while(!fine_turno) {
                printf("1)Avanti 2)Indietro 3)Mondo 4)Combatti 5)Passa 6)Raccogli un oggetto 7)Usa un'oggetto\nScelta: ");
                int az;
                if (scanf("%d", &az) != 1)
                {
                  while(getchar()!='\n');
                  continue;
                }
                int esito_combat = 0;
                switch(az) {
                    case 1:
                    avanza(g, &ha_mosso);
                    break;
                    case 2:
                    indietreggia(g);
                    break;
                    case 3:
                    cambia_mondo(g, &ha_mosso);
                    break;
                    case 4:
                        esito_combat = combatti(g);
                        if (esito_combat == 2)
                        { // Vittoria finale
                            puts("\n*** HAI VINTO! DEMOTORZONE SCONFITTO! ***");
                            registra_vincitore(g->nome);
                            vittoria = 1;
                            fine_turno = 1; // Esce dal while turno
                        } else if (esito_combat == 1)
                        { // Morte giocatore
                            printf("!!! %s E' MORTO !!!\n", g->nome);
                            free(giocatori[ind]);
                            giocatori[ind] = NULL;
                            fine_turno = 1; // Esce dal while turno
                        }
                        break;
                    case 5:
                    fine_turno = 1;
                    break;
                    case 6:
                    raccogli_oggetto(g);
                    break;
                    case 7:
                    usa_oggetto(g);
                    break;
                    default:
                    puts("No.");
                }
                if (vittoria || giocatori[ind] == NULL)
                break; // Esce dal while azioni
            }
            if (vittoria)
            break; // Esce dal for giocatori
        }

        if (conta_vivi == 0 && !vittoria) {
            puts("TUTTI MORTI - GAME OVER.");
            tutti_morti = 1;
        }
    }
}

void termina_gioco() {
    puts("Pulizia memoria in corso...");
    int i;
    for(i=0; i<4; i++)
    {
        if (giocatori[i]) free(giocatori[i]);
    }
    pulizia_completa_mappa();
    puts("Arrivederci");
}

/*-------------------------
 |      MAPPA             |
 -------------------------*/
static void genera_mappa() {
    pulizia_completa_mappa(); // Pulisce SEMPRE prima di creare

    struct Zona_mondoreale* last_r = NULL;
    struct Zona_soprasotto* last_s = NULL;

    int i;
    for (i = 0; i < 15; i++)
    {
        struct Zona_mondoreale* nr = malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto* ns = malloc(sizeof(struct Zona_soprasotto));

        if (!nr || !ns)
        {
          puts("Memoria piena");
          exit(1);
        }

        nr->tipo = random_tipo(); nr->nemico = random_nemico(0); nr->oggetto = random_oggetto();
        ns->tipo = nr->tipo;      ns->nemico = random_nemico(1);

        nr->link_soprasotto = ns; ns->link_mondoreale = nr;
        nr->avanti = NULL; ns->avanti = NULL;

        if (i == 0)
        {
            prima_zona_mondoreale = nr; prima_zona_soprasotto = ns;
            nr->indietro = NULL; ns->indietro = NULL;
        } else {
            last_r->avanti = nr; nr->indietro = last_r;
            last_s->avanti = ns; ns->indietro = last_s;
        }
        last_r = nr; last_s = ns;
    }
    last_s->nemico = demotorzone;
    puts("Mappa generata.");
}

static void inserisci_zona()
{
    if (!prima_zona_mondoreale)
    {
      puts("Non è stata creata la mappa");
      return;
    }
    int p;
    printf("Posizione: ");
    if (scanf("%d", &p) != 1)
    {
        while(getchar() != '\n'); // Pulisce il buffer sporco
        puts("Errore: devi inserire un numero! Inserimento annullato.");
        return; // Torna al menu
    }

    struct Zona_mondoreale* nr = malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* ns = malloc(sizeof(struct Zona_soprasotto));

    nr->tipo = random_tipo();
    nr->nemico = random_nemico(0);
    nr->oggetto = random_oggetto();
    ns->tipo = nr->tipo;
    ns->nemico = random_nemico(1);
    nr->link_soprasotto = ns;
    ns->link_mondoreale = nr;

    if (p <= 0)
    {
        nr->avanti = prima_zona_mondoreale; prima_zona_mondoreale->indietro = nr;
         nr->indietro = NULL;
        ns->avanti = prima_zona_soprasotto; prima_zona_soprasotto->indietro = ns;
         ns->indietro = NULL;
        prima_zona_mondoreale = nr; prima_zona_soprasotto = ns;
    } else {
        struct Zona_mondoreale* curr = prima_zona_mondoreale;
        struct Zona_soprasotto* curr_s = prima_zona_soprasotto;
        int c = 0;
        while (c < p-1 && curr->avanti)
        {
          curr = curr->avanti;
          curr_s = curr_s->avanti;
          c++;
        }
        nr->avanti = curr->avanti;
        nr->indietro = curr; curr->avanti = nr;
        ns->avanti = curr_s->avanti;
        ns->indietro = curr_s;
        curr_s->avanti = ns;

        if (nr->avanti)
        {
           nr->avanti->indietro = nr;
           ns->avanti->indietro = ns;
        }
    }
    puts("Ok.");
}

static void cancella_zona()
{
    if (!prima_zona_mondoreale)
    {
      puts("Mappa vuota, niente da cancellare");
      return;
    }
    int p;
    printf("Inserisci posizione della zona da cancellare: ");

    if(scanf("%d", &p) != 1)
    {
      while(getchar() != '\n'); // Pulisce il buffer
      puts("Errore: Input non valido. Cancellazione annullata.");
      return;
    }

    struct Zona_mondoreale* d = prima_zona_mondoreale;
    struct Zona_soprasotto* d_s = prima_zona_soprasotto;
    int c = 0;
    while (d && c < p)
    {
      d = d->avanti;
      d_s = d_s->avanti;
      c++;
    }

    if (!d)
      return;

    if (d == prima_zona_mondoreale)
    {
        prima_zona_mondoreale = d->avanti;
        prima_zona_soprasotto = d_s->avanti;
        if (prima_zona_mondoreale)
        {
          prima_zona_mondoreale->indietro = NULL;
          prima_zona_soprasotto->indietro = NULL;
        }
    } else
    {
        d->indietro->avanti = d->avanti;
        d_s->indietro->avanti = d_s->avanti;
        if (d->avanti)
        {
          d->avanti->indietro = d->indietro;
          d_s->avanti->indietro = d_s->indietro;
        }
    }
    free(d);
    free(d_s);
    puts("Eliminata");
}

static void stampa_mappa()
{
    struct Zona_mondoreale* r = prima_zona_mondoreale;
    int i=1;
    while(r)
    {
        printf("%d) %s [Nemico: %s]\n", i++, nome_zona(r->tipo), nome_nemico(r->nemico));
        r = r->avanti;
    }
}

static void chiudi_mappa() {
    if (!prima_zona_mondoreale)
    {
      puts("Vuota.");
      return;
    }

    int conteggio_zone = 0;
    int conteggio_demotorzone = 0;

    struct Zona_mondoreale* corr_r = prima_zona_mondoreale;
    while(corr_r !=NULL)
    {
      conteggio_zone++;
      corr_r  =corr_r->avanti;
    }

    struct Zona_soprasotto* corr_s = prima_zona_soprasotto;
    while(corr_s !=NULL)
    {
      if(corr_s->nemico == demotorzone)
      {
        conteggio_demotorzone++;
      }
      corr_s = corr_s->avanti;
    }


    if (conteggio_zone < 15)
    {
        printf("Errore: la mappa ha solo %d zone,ne servono almeno 15.\n", conteggio_zone);
    } else if (conteggio_demotorzone != 1) {
        printf("Errore: deve esserci esattamente 1 Demotorzone (presenti: %d).\n", conteggio_demotorzone);
    } else {
        mappa_chiusa = 1;
        puts("Mappa convalidata! Ora puoi selezionare 'Gioca' dal menu principale.");
    }
}

void crediti() {
    puts("   SVILUPPATO DA: MOHAMED IYED ABIDI");
    puts(" MATRICOLA: 394197 ");
    puts("   Progetto Esame C - Cosestrane");

    if (n_vincitori == 0) {
        puts("Nessun vincitore registrato");
    } else {
        puts("\n--- Ultimi Vincitori ---");
        int i;
        for (i = 0; i < n_vincitori; i++) {
            printf("%d) %s\n", i + 1, vincitori[i]);
        }
    }
}
