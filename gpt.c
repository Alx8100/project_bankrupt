#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { HIT, STAND, DOUBLE, SPLIT } Action;

int cardValue(char card) {
    if (card == 'A') return 11; // Potrebbe valere anche 1, gestito separatamente
    if (card == 'K' || card == 'Q' || card == 'J' || card == 'T') return 10;
    return card - '0';
}

int hi_lo(char card) {
    if (card == 'A' || card == 'K' || card == 'Q' || card == 'J' || card == 'T')
        return -1;
    if ('2' <= card && card <= '6')
        return 1;
    return 0;
}

Action getAction(int playerScore, int dealerCard, int isSoft, int canSplit, int playerCard1, int playerCard2) {
    if (canSplit && playerCard1 == playerCard2) {
        if (playerCard1 == 8 || playerCard1 == 11) return SPLIT;
        if (playerCard1 == 9 && dealerCard >= 2 && dealerCard <= 9 && dealerCard != 7) return SPLIT;
        if (playerCard1 == 7 && dealerCard >= 2 && dealerCard <= 7) return SPLIT;
        if (playerCard1 == 6 && dealerCard >= 2 && dealerCard <= 6) return SPLIT;
        if (playerCard1 == 3 && dealerCard >= 2 && dealerCard <= 7) return SPLIT;
        if (playerCard1 == 2 && dealerCard >= 2 && dealerCard <= 7) return SPLIT;
        if (playerCard1 == 4 && (dealerCard == 5 || dealerCard == 6)) return SPLIT;
    }

    if (isSoft) {
        if (playerScore == 18 && (dealerCard == 2 || dealerCard == 7 || dealerCard == 8)) return STAND;
        if (playerScore == 18 && dealerCard >= 3 && dealerCard <= 6) return DOUBLE;
        if (playerScore == 18 && dealerCard >= 9) return HIT;
        if (playerScore >= 19) return STAND;
        if (playerScore == 17 && dealerCard >= 3 && dealerCard <= 6) return DOUBLE;
        if (playerScore <= 17) return HIT;
    } else {
        if (playerScore >= 17) return STAND;
        if (playerScore >= 13 && playerScore <= 16 && dealerCard >= 2 && dealerCard <= 6) return STAND;
        if (playerScore == 12 && dealerCard >= 4 && dealerCard <= 6) return STAND;
        if (playerScore == 11 || (playerScore == 10 && dealerCard <= 9) || (playerScore == 9 && dealerCard >= 3 && dealerCard <= 6)) return DOUBLE;
        if (playerScore <= 11) return HIT;
    }

    return HIT;
}

// Calcola la frazione di Kelly per determinare la puntata
double kellyFraction(int cardCount) {
    double advantage = cardCount * 0.1; // Stimare un vantaggio dello 0.5% per punto del conteggio
    return advantage > 0 ? advantage : 0; // Assicurarsi che il vantaggio non sia negativo
}

// Determina la puntata usando il criterio di Kelly
int determineBet(int baseBet, double kellyFraction, int saldo) {
    int bet = (int)(kellyFraction * saldo);
    if (bet < baseBet) bet = baseBet; // Puntata minima
    if (bet > saldo) bet = saldo; // Non scommettere più del saldo disponibile
    return bet;
}

// Aggiorna il punteggio del giocatore gestendo correttamente l'asso
void updatePlayerScore(int *playerScore, int cardValue, int *isSoft) {
    *playerScore += cardValue;
    if (cardValue == 11) {
        *isSoft = 1;
    }
    if (*playerScore > 21 && *isSoft) {
        *playerScore -= 10;
        *isSoft = 0;
    }
}

int main() {
    char playerCards[5], dealerCard, seenCards[52];
    int cardCount = 0;
    int baseBet;
    int saldo;

    // Chiedi al giocatore il saldo iniziale
    printf("Inserisci il saldo iniziale: ");
    scanf("%d", &saldo);

    // Chiedi al giocatore il conteggio delle carte attuale
    printf("Inserisci il current card count: ");
    scanf("%d", &cardCount);

    // Chiedi al giocatore la puntata minima
    printf("Inserisci la puntata minima: ");
    scanf("%d", &baseBet);

    while (1) {
        int dealerCardValue, playerCard1Value, playerCard2Value;
        int isSoft = 0, canSplit = 0, playerScore = 0;

        // Calcola la frazione di Kelly e determina la puntata suggerita
        double kelly = kellyFraction(cardCount);
        int currentBet = determineBet(baseBet, kelly, saldo);

        // Mostra il conteggio delle carte, il saldo attuale e il suggerimento di puntata
        printf("Current card count: %d\n", cardCount);
        printf("Suggested bet: %d\n", currentBet);
        printf("Saldo attuale: %d\n", saldo);

        // Input delle carte del giocatore
        printf("Inserisci le carte del giocatore (es. 8K per 8 e K): ");
        scanf("%s", playerCards);

        // Input della carta del dealer
        printf("Inserisci la carta scoperta del dealer (es. K): ");
        scanf(" %c", &dealerCard);

        // Calcolo del valore delle carte del giocatore
        playerCard1Value = cardValue(playerCards[0]);
        playerCard2Value = cardValue(playerCards[1]);
        playerScore = playerCard1Value + playerCard2Value;
        if (playerCards[0] == 'A' || playerCards[1] == 'A') isSoft = 1;
        if (playerCards[0] == playerCards[1]) canSplit = 1;

        // Calcolo del valore della carta del dealer
        dealerCardValue = cardValue(dealerCard);

        // Aggiorna il conteggio delle carte con le carte del giocatore e del dealer
        cardCount += hi_lo(playerCards[0]);
        cardCount += hi_lo(playerCards[1]);
        cardCount += hi_lo(dealerCard);

        // Gestisce la mano del giocatore fino a STAND o BUST
        while (1) {
            Action action = getAction(playerScore, dealerCardValue, isSoft, canSplit, playerCard1Value, playerCard2Value);
            if (action == STAND || playerScore > 21) {
                break;
            }

            // Se HIT, aggiungi una carta
            if (action == HIT) {
                printf("Player should HIT.\n");
                printf("Inserisci la prossima carta del giocatore: ");
                char nextCard;
                scanf(" %c", &nextCard);
                int nextCardValue = cardValue(nextCard);
                updatePlayerScore(&playerScore, nextCardValue, &isSoft);
                cardCount += hi_lo(nextCard);
            } else if (action == DOUBLE) {
                printf("Player should DOUBLE.\n");
                currentBet *= 2;
                printf("Inserisci la prossima carta del giocatore: ");
                char nextCard;
                scanf(" %c", &nextCard);
                int nextCardValue = cardValue(nextCard);
                updatePlayerScore(&playerScore, nextCardValue, &isSoft);
                cardCount += hi_lo(nextCard);
                break; // Dopo DOUBLE si deve fermare
            } else if (action == SPLIT) {
                // Logica di gestione dello split
                printf("Player should SPLIT.\n");
                // Gestiamo lo split manualmente per ora
                // Dividiamo le due carte
                char firstHandCard1 = playerCards[0];
                char secondHandCard1 = playerCards[1];

                // Prima mano
                printf("Gestione della prima mano con carta %c.\n", firstHandCard1);
                playerScore = cardValue(firstHandCard1);
                isSoft = (firstHandCard1 == 'A') ? 1 : 0;

                while (1) {
                    printf("Inserisci la prossima carta della prima mano: ");
                    char nextCard;
                    scanf(" %c", &nextCard);
                    int nextCardValue = cardValue(nextCard);
                    updatePlayerScore(&playerScore, nextCardValue, &isSoft);
                    cardCount += hi_lo(nextCard);
                    action = getAction(playerScore, dealerCardValue, isSoft, 0, firstHandCard1, nextCard);
                    if (action == STAND || playerScore > 21) {
                        break;
                    }
                    if (action == DOUBLE) {
                        printf("Player should DOUBLE.\n");
                        currentBet *= 2;
                        printf("Inserisci la prossima carta della prima mano: ");
                        scanf(" %c", &nextCard);
                        nextCardValue = cardValue(nextCard);
                        updatePlayerScore(&playerScore, nextCardValue, &isSoft);
                        cardCount += hi_lo(nextCard);
                        break; // Dopo DOUBLE si deve fermare
                    }
                }
                printf("Prima mano: Player stands with score %d.\n", playerScore);

                // Seconda mano
                printf("Gestione della seconda mano con carta %c.\n", secondHandCard1);
                playerScore = cardValue(secondHandCard1);
                isSoft = (secondHandCard1 == 'A') ? 1 : 0;

                while (1) {
                    printf("Inserisci la prossima carta della seconda mano: ");
                    char nextCard;
                    scanf(" %c", &nextCard);
                    int nextCardValue = cardValue(nextCard);
                    updatePlayerScore(&playerScore, nextCardValue, &isSoft);
                    cardCount += hi_lo(nextCard);
                    action = getAction(playerScore, dealerCardValue, isSoft, 0, secondHandCard1, nextCard);
                    if (action == STAND || playerScore > 21) {
                        break;
                    }
                    if (action == DOUBLE) {
                        printf("Player should DOUBLE.\n");
                        currentBet *= 2;
                        printf("Inserisci la prossima carta della seconda mano: ");
                        scanf(" %c", &nextCard);
                        nextCardValue = cardValue(nextCard);
                        updatePlayerScore(&playerScore, nextCardValue, &isSoft);
                        cardCount += hi_lo(nextCard);
                        break; // Dopo DOUBLE si deve fermare
                    }
                }
                printf("Seconda mano: Player stands with score %d.\n", playerScore);
                break; // Uscita dopo la gestione dello split
            }
        }

        // Output del risultato del giocatore
        if (playerScore > 21) {
            printf("Player busts with score %d.\n", playerScore);
        } else {
            printf("Player stands with score %d.\n", playerScore);
        }

        // Input delle carte già uscite
        printf("Inserisci le carte già uscite (es. 23456QJT): ");
        scanf("%s", seenCards);

        // Aggiorna il conteggio delle carte con le carte già uscite
        for (int i = 0; i < strlen(seenCards); i++) {
            cardCount += hi_lo(seenCards[i]);
        }

        // Aggiorna il saldo in base al risultato della mano
        char outcome;
        printf("Risultato della mano (w per win, l per loss, p per push): ");
        scanf(" %c", &outcome);
        if (outcome == 'w' || outcome == 'W') {
            saldo += currentBet;
        } else if (outcome == 'l' || outcome == 'L') {
            saldo -= currentBet;
        }

        // Stampa il conteggio delle carte aggiornato
        printf("Updated card count: %d\n", cardCount);
        printf("Saldo aggiornato: %d\n", saldo);

        // Chiedi se il giocatore vuole giocare un'altra mano
        char playAgain;
        if (saldo > 0) {
            printf("Vuoi giocare un'altra mano? (s/n): ");
            scanf(" %c", &playAgain);
            if (playAgain != 's' && playAgain != 'S') {
                break;
            }
        } else {
            printf("Saldo esaurito! Non puoi continuare a giocare.\n");
            break;
        }
    }

    return 0;
}
