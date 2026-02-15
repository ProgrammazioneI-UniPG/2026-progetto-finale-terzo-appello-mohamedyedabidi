#include <stdio.h>
#include "gamelib.h"
int main(){
  int scelta;
  do{
    printf("---- MENU DI GIOCO ----\n");
    printf("1) Imposta gioco\n");
    printf("2) Gioca\n");
    printf("3) Termina gioco\n");
    printf("4) Visualizza i crediti\n");
    printf ("Scegli una delle seguenti modalita': ");
    if(scanf("%d", &scelta) != 1)
    {
      while (getchar() != '\n');
      scelta = 0;         //imposto una scelta non valida cosi ristampa il menu
      continue;           //ricomincia da capo il codice
    }
    switch(scelta)
    {
      case 1:
        imposta_gioco();
        break;
      case 2:
        gioca();
        break;
      case 3:
        termina_gioco();
        break;
      case 4:
        crediti();
        break;
      default:
        puts("Scelta non valida");
        break;
    }
  }while(scelta != 3);
    return 0;


}
