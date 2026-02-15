#ifndef GAMELIB_H
#define GAMELIB_H
enum Tipo_zona
{
  bosco,
  scuola,
  laboratorio,
  caverna,
  strada,
  giardino,
  supermercato,
  centrale_elettrica,
  deposito_abbandonato,
  stazione_polizia
};
enum Tipo_nemico
{
  nessun_nemico,
  billi,
  democane,
  demotorzone
};
enum Tipo_oggetto
{
  nessun_oggetto,
  bicicletta,
  maglietta_fuocoinferno,
  bussola,
  schitarrata_metallica
};
//dichiaro i nodi qui per far in modo i link si riconoscano tra loro
struct Zona_soprasotto;
struct Zona_mondoreale;

struct Zona_soprasotto
{
  enum Tipo_zona tipo;
  enum Tipo_nemico nemico;

  struct Zona_soprasotto* avanti;
  struct Zona_soprasotto* indietro;

  struct Zona_mondoreale* link_mondoreale;
};


struct Zona_mondoreale
{
  enum Tipo_zona tipo;
  enum Tipo_nemico nemico;
  enum Tipo_oggetto oggetto;
  struct Zona_mondoreale* avanti;
  struct Zona_mondoreale* indietro;
  struct Zona_soprasotto* link_soprasotto;
};


struct Giocatore
{
char nome[30];
int mondo;
struct Zona_mondoreale* pos_mondoreale;
struct Zona_soprasotto* pos_soprasotto;
int attacco_pischico;
int difesa_pischica;
int fortuna;
enum Tipo_oggetto zaino[3];
};

void imposta_gioco();
void gioca();
void termina_gioco();
void crediti();

#endif
