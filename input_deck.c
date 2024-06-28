#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { HIT, STAND, DOUBLE, SPLIT } Action;

int cardValue(char card) {
    if (card == 'A') return 11;
    if (card == 'K' || card == 'Q' || card == 'J' || card == 'T') return 10;
    return card - '0';
}

double wong_halves(char card) {
    if (card == 'A' || card == 'K' || card == 'Q' || card == 'J' || card == 'T') return -1;
    if (card == '9') return -0.5;
    if (card == '8') return 0;
    if (card == '7') return 0.5;
    if (card == '6' || card == '3') return 1;
    if (card == '4' || card == '5') return 1.5;
    if (card == '2') return 0.5;
    return 0;
}

Action getAction(int playerScore, int dealerCard, int isSoft, int canSplit, int playerCard1, int playerCard2, double trueCount) {
    if (!isSoft) {
        if (playerScore == 16 && dealerCard == 10 && trueCount >= 0) return STAND;
        if (playerScore == 15 && dealerCard == 10 && trueCount >= 4) return STAND;
        if (playerScore == 13 && dealerCard == 2 && trueCount < 0) return HIT;
        if (playerScore == 12 && dealerCard == 3 && trueCount < 2) return HIT;
        if (playerScore == 12 && dealerCard == 2 && trueCount < 3) return HIT;
    }
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

double kellyFraction(double trueCount) {
    double advantage = trueCount * 0.005;
    return advantage > 0 ? advantage : 0;
}

int determineBet(int baseBet, double kellyFraction, int balance) {
    int bet = (int)(kellyFraction * balance);
    if (bet < baseBet) bet = baseBet;
    if (bet > balance) bet = balance;
    return bet;
}

int determineMinBet(int balance) {
    return balance * 0.01;
}

void updatePlayerScore(int *playerScore, int cardValue, int *isSoft, int *numAces) {
    *playerScore += cardValue;
    if (cardValue == 11) {
        (*numAces)++;
        *isSoft = 1;
    }
    while (*playerScore > 21 && *numAces > 0) {
        *playerScore -= 10;
        (*numAces)--;
    }
    if (*numAces == 0) {
        *isSoft = 0;
    }
}

// Function prototype for handleSplitHands
void handleSplitHands(char *playerCards, int dealerCardValue, double trueCount, int *currentBet, int *playerScore, double *cardCount, int *isSoft, int *numAces);

void initializeGame(int *decks, int *balance, int *baseBet, double *cardCount) {
    printf("Enter the number of decks: ");
    scanf("%i", decks);

    printf("Enter the initial balance: ");
    scanf("%d", balance);

    int minBet = determineMinBet(*balance);
    printf("Minimum bet based on your balance: %d\n", minBet);

    printf("Enter the minimum bet (should be at least %d): ", minBet);
    scanf("%d", baseBet);
    if (*baseBet < minBet) {
        printf("The minimum bet should be at least %d. Setting base bet to %d.\n", minBet, minBet);
        *baseBet = minBet;
    }

    printf("Enter the current card count: ");
    scanf("%lf", cardCount);
}

void getPlayerCards(char *playerCards) {
    printf("Enter the player's cards (e.g., 8K for 8 and K): ");
    scanf("%s", playerCards);
    if (strlen(playerCards) != 2) {
        printf("Invalid input. Please enter exactly two cards.\n");
        getPlayerCards(playerCards);
    }
}

void getDealerCard(char *dealerCard) {
    printf("Enter the dealer's up card (e.g., K): ");
    scanf(" %c", dealerCard);
    if (strchr("23456789TJQKA", *dealerCard) == NULL) {
        printf("Invalid input. Please enter a valid card.\n");
        getDealerCard(dealerCard);
    }
}

double calculateTrueCount(double cardCount, int decks, char *seenCards) {
    int remainingDecks = decks - (int)strlen(seenCards) / 52;
    return (double)cardCount / remainingDecks;
}

void displayGameStatus(double cardCount, double trueCount, int currentBet, int balance) {
    printf("Current card count: %f\n", cardCount);
    printf("True count: %.2f\n", trueCount);
    printf("Suggested bet: %d\n", currentBet);
    printf("Current balance: %d\n", balance);
}

void updateScores(int *playerScore, char *playerCards, int *isSoft, int *numAces, int *canSplit) {
    int playerCard1Value = cardValue(playerCards[0]);
    int playerCard2Value = cardValue(playerCards[1]);
    if (playerCard1Value == 11 && playerCard2Value == 11) {
        *playerScore = 12;
        *numAces = 1;
        *isSoft = 1;
    } else {
        *playerScore = playerCard1Value + playerCard2Value;
        if (playerCards[0] == 'A') (*numAces)++;
        if (playerCards[1] == 'A') (*numAces)++;
        *isSoft = (*numAces > 0);
    }
    if (playerCards[0] == playerCards[1]) *canSplit = 1;
}

void updateCardCount(double *cardCount, char *playerCards, char dealerCard) {
    *cardCount += wong_halves(playerCards[0]);
    *cardCount += wong_halves(playerCards[1]);
    *cardCount += wong_halves(dealerCard);
}

void handlePlayerHand(int *playerScore, int *isSoft, int *numAces, double *cardCount, char *playerCards, int dealerCardValue, double trueCount, int *currentBet) {
    int playerCard1Value = cardValue(playerCards[0]);
    int playerCard2Value = cardValue(playerCards[1]);
    int canSplit = 0;
    if (playerCards[0] == playerCards[1]) canSplit = 1;

    while (1) {
        Action action = getAction(*playerScore, dealerCardValue, *isSoft, canSplit, playerCard1Value, playerCard2Value, trueCount);
        if (action == STAND || *playerScore > 21) {
            break;
        }
        if (action == HIT) {
            printf("Player should HIT.\n");
            printf("Enter the next card for the player: ");
            char nextCard;
            scanf(" %c", &nextCard);
            int nextCardValue = cardValue(nextCard);
            updatePlayerScore(playerScore, nextCardValue, isSoft, numAces);
            *cardCount += wong_halves(nextCard);
        } else if (action == DOUBLE) {
            printf("Player should DOUBLE.\n");
            *currentBet *= 2;
            printf("Enter the next card for the player: ");
            char nextCard;
            scanf(" %c", &nextCard);
            int nextCardValue = cardValue(nextCard);
            updatePlayerScore(playerScore, nextCardValue, isSoft, numAces);
            *cardCount += wong_halves(nextCard);
            break;
        } else if (action == SPLIT) {
            printf("Player should SPLIT.\n");
            handleSplitHands(playerCards, dealerCardValue, trueCount, currentBet, playerScore, cardCount, isSoft, numAces);
            break;
        }
    }
}

void handleSplitHands(char *playerCards, int dealerCardValue, double trueCount, int *currentBet, int *playerScore, double *cardCount, int *isSoft, int *numAces) {
    char firstHandCard1 = playerCards[0];
    char secondHandCard1 = playerCards[1];

    // First hand
    printf("Handling first hand with card %c.\n", firstHandCard1);
    *playerScore = cardValue(firstHandCard1);
    *isSoft = (firstHandCard1 == 'A') ? 1 : 0;
    *numAces = (firstHandCard1 == 'A') ? 1 : 0;
    while (1) {
        printf("Enter the next card for the first hand: ");
        char nextCard;
        scanf(" %c", &nextCard);
        int nextCardValue = cardValue(nextCard);
        updatePlayerScore(playerScore, nextCardValue, isSoft, numAces);
        *cardCount += wong_halves(nextCard);
        Action action = getAction(*playerScore, dealerCardValue, *isSoft, 0, firstHandCard1, nextCard, trueCount);
        if (action == STAND || *playerScore > 21) {
            break;
        }
        if (action == DOUBLE) {
            printf("Player should DOUBLE.\n");
            *currentBet *= 2;
            printf("Enter the next card for the first hand: ");
            scanf(" %c", &nextCard);
            nextCardValue = cardValue(nextCard);
            updatePlayerScore(playerScore, nextCardValue, isSoft, numAces);
            *cardCount += wong_halves(nextCard);
            break;
        }
    }
    printf("First hand: Player stands with score %d.\n", *playerScore);

    // Second hand
    printf("Handling second hand with card %c.\n", secondHandCard1);
    *playerScore = cardValue(secondHandCard1);
    *isSoft = (secondHandCard1 == 'A') ? 1 : 0;
    *numAces = (secondHandCard1 == 'A') ? 1 : 0;
    while (1) {
        printf("Enter the next card for the second hand: ");
        char nextCard;
        scanf(" %c", &nextCard);
        int nextCardValue = cardValue(nextCard);
        updatePlayerScore(playerScore, nextCardValue, isSoft, numAces);
        *cardCount += wong_halves(nextCard);
        Action action = getAction(*playerScore, dealerCardValue, *isSoft, 0, secondHandCard1, nextCard, trueCount);
        if (action == STAND || *playerScore > 21) {
            break;
        }
        if (action == DOUBLE) {
            printf("Player should DOUBLE.\n");
            *currentBet *= 2;
            printf("Enter the next card for the second hand: ");
            scanf(" %c", &nextCard);
            nextCardValue = cardValue(nextCard);
            updatePlayerScore(playerScore, nextCardValue, isSoft, numAces);
            *cardCount += wong_halves(nextCard);
            break;
        }
    }
    printf("Second hand: Player stands with score %d.\n", *playerScore);
}

void getSeenCards(char *seenCards, double *cardCount) {
    printf("Enter the cards that have been played (e.g., 23456QJT): ");
    scanf("%s", seenCards);
    for (int i = 0; i < strlen(seenCards); i++) {
        *cardCount += wong_halves(seenCards[i]);
    }
}

void updateBalance(int *balance, int currentBet) {
    char outcome;
    printf("Result of the hand (w for win, l for loss, p for push): ");
    scanf(" %c", &outcome);
    if (outcome == 'w' || outcome == 'W') {
        *balance += currentBet;
    } else if (outcome == 'l' || outcome == 'L') {
        *balance -= currentBet;
    }
}

int playAgain() {
    char playAgain;
    printf("Do you want to play another hand? (y/n): ");
    scanf(" %c", &playAgain);
    return (playAgain == 'y' || playAgain == 'Y');
}

int main() {
    char playerCards[5], dealerCard, seenCards[416] = {0}; // 416 cards in 8 decks
    double cardCount = 0;
    int baseBet;
    int balance;
    int decks;

    initializeGame(&decks, &balance, &baseBet, &cardCount);

    while (balance > 0) {
        int dealerCardValue, playerScore = 0;
        int isSoft = 0, numAces = 0;
        int canSplit = 0;

        double trueCount = calculateTrueCount(cardCount, decks, seenCards);
        int currentBet = determineBet(baseBet, kellyFraction(trueCount), balance);

        displayGameStatus(cardCount, trueCount, currentBet, balance);

        getPlayerCards(playerCards);
        getDealerCard(&dealerCard);

        updateScores(&playerScore, playerCards, &isSoft, &numAces, &canSplit);
        dealerCardValue = cardValue(dealerCard);
        updateCardCount(&cardCount, playerCards, dealerCard);

        handlePlayerHand(&playerScore, &isSoft, &numAces, &cardCount, playerCards, dealerCardValue, trueCount, &currentBet);

        if (playerScore > 21) {
            printf("Player busts with score %d.\n", playerScore);
        } else {
            printf("Player stands with score %d.\n", playerScore);
        }

        getSeenCards(seenCards, &cardCount);
        updateBalance(&balance, currentBet);

        if (!playAgain()) break;
    }

    printf("Game over! Final balance: %d\n", balance);
    return 0;
}
