#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { HIT, STAND, DOUBLE, SPLIT } Action;

const char *cards = "A23456789TJQK";

int cardValue(char card) {
    if (card == 'A') return 11; // Ace can be 1 or 11
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

Action getAction(int playerScore, int dealerCard, int isSoft, int canSplit, int playerCard1, int playerCard2, double trueCount, int hasHit) {
    // Implementare le deviazioni di gioco
    if (!isSoft) {
        if (playerScore == 16 && dealerCard == 10 && trueCount >= 0) return STAND; // Stand su 16 contro 10 con true count >= 0
        if (playerScore == 15 && dealerCard == 10 && trueCount >= 4) return STAND; // Stand su 15 contro 10 con true count >= 4
        if (playerScore == 13 && dealerCard == 2 && trueCount < 0) return HIT; // Hit su 13 contro 2 con true count < 0
        if (playerScore == 12 && dealerCard == 3 && trueCount < 2) return HIT; // Hit su 12 contro 3 con true count < 2
        if (playerScore == 12 && dealerCard == 2 && trueCount < 3) return HIT; // Hit su 12 contro 2 con true count < 3
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
        if ((playerScore == 11 || (playerScore == 10 && dealerCard <= 9) || (playerScore == 9 && dealerCard >= 3 && dealerCard <= 6)) && !hasHit) return DOUBLE;
        if (playerScore <= 11) return HIT;
    }

    return HIT;
}

// Calculate Kelly fraction to determine the bet
double kellyFraction(double trueCount) {
    double advantage = trueCount * 0.005; // Estimating a 0.5% advantage per true count point
    return advantage > 0 ? advantage : 0; // Ensure the advantage is not negative
}

// Determine the bet using the Kelly Criterion
int determineBet(int baseBet, double kellyFraction, int balance) {
    int bet = (int)(kellyFraction * balance);
    if (bet < baseBet) bet = baseBet; // Minimum bet
    if (bet > balance) bet = balance; // Don't bet more than the available balance
    return bet;
}

// Determine the minimum bet based on conservative strategy
int determineMinBet(int balance) {
    return balance * 0.01; // Minimum bet as 1% of the balance
}

// Update player's score, correctly handling the ace
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

// Initialize the deck
void initializeDeck(char *deck, int decks) {
    int i;
    for (i = 0; i < decks * 52; i++) {
        deck[i] = cards[i % 13];
    }
}

// Shuffle the deck
void shuffleDeck(char *deck, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

// Draw a card from the deck
char drawCard(char *deck, int *index) {
    return deck[(*index)++];
}

int simulateHands(int initialBalance, int baseBet, int numHands, int decks) {
    char deck[decks * 52];
    int deckIndex = 0;
    initializeDeck(deck, decks);
    shuffleDeck(deck, decks * 52);

    double cardCount = 0;
    int balance = initialBalance;

    for (int hand = 0; hand < numHands; hand++) {
        if (deckIndex > decks * 52 - 15) { // Reshuffle if deck is low
            initializeDeck(deck, decks);
            shuffleDeck(deck, decks * 52);
            deckIndex = 0;
        }

        int dealerCardValue, playerCard1Value, playerCard2Value;
        int isSoft = 0, canSplit = 0, playerScore = 0, numAces = 0;
        char playerCards[3] = { drawCard(deck, &deckIndex), drawCard(deck, &deckIndex), '\0' };
        char dealerCard = drawCard(deck, &deckIndex);

        // Calculate the value of the player's cards
        playerCard1Value = cardValue(playerCards[0]);
        playerCard2Value = cardValue(playerCards[1]);
        playerScore = playerCard1Value + playerCard2Value;
        if (playerCard1Value == 11 || playerCard2Value == 11) isSoft = 1;
        if (playerCard1Value == playerCard2Value) canSplit = 1;

        // Calculate true count
        int remainingDecks = decks - deckIndex / 52;
        double trueCount = (double)cardCount / remainingDecks;

        // Calculate Kelly fraction and determine the suggested bet
        double kelly = kellyFraction(trueCount);
        int currentBet = determineBet(baseBet, kelly, balance);

        dealerCardValue = cardValue(dealerCard);

        cardCount += wong_halves(playerCards[0]);
        cardCount += wong_halves(playerCards[1]);
        cardCount += wong_halves(dealerCard);

        // Check for insurance
        if (dealerCard == 'A') {
            int insuranceAdvice = (trueCount >= 3) ? 1 : 0;
            printf("Insurance advice: %s\n", insuranceAdvice ? "Take insurance" : "Do not take insurance");
        }

        int hasHit = 0; // Initialize hasHit to track if the player has hit

        // Simulate player's actions until STAND or BUST
        while (1) {
            Action action = getAction(playerScore, dealerCardValue, isSoft, canSplit, playerCard1Value, playerCard2Value, trueCount, hasHit);
            if (action == STAND || playerScore > 21) {
                break;
            }

            if (action == HIT) {
                char nextCard = drawCard(deck, &deckIndex);
                int nextCardValue = cardValue(nextCard);
                updatePlayerScore(&playerScore, nextCardValue, &isSoft, &numAces);
                cardCount += wong_halves(nextCard);
                hasHit = 1; // Mark that the player has hit
            } else if (action == DOUBLE && !hasHit) {
                currentBet *= 2;
                char nextCard = drawCard(deck, &deckIndex);
                int nextCardValue = cardValue(nextCard);
                updatePlayerScore(&playerScore, nextCardValue, &isSoft, &numAces);
                cardCount += wong_halves(nextCard);
                break;
            } else if (action == SPLIT) {
                // Handle split manually for now
                break; // For simplicity, skip actual split logic in this test
            }
        }

        // Simulate the result of the hand
        char outcome = (playerScore <= 21 && playerScore > 17) ? 'w' : 'l'; // Simplified outcome logic
        if (outcome == 'w') {
            balance += currentBet;
        } else if (outcome == 'l') {
            balance -= currentBet;
        }
    }

    return balance;
}

// Main function for testing
int main() {
    srand(time(NULL)); // Initialize random seed

    int initialBalance = 1000; // Example initial balance
    int baseBet = 10;           // Example base bet
    int numHands = 100;        // Example number of hands to simulate
    int decks = 8;             // Fixed number of decks, 4 in play

    int finalBalance = simulateHands(initialBalance, baseBet, numHands, decks);
    printf("Final balance after %d hands: %d\n", numHands, finalBalance);

    return 0;
}
