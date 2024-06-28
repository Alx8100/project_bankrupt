#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { HIT, STAND, DOUBLE, SPLIT } Action;

int cardValue(char card) {
    if (card == 'A') return 11; // Ace can be 1 or 11
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

int main() {
    char playerCards[5], dealerCard, seenCards[52] = {0};
    int cardCount = 0;
    int baseBet;
    int balance;
    int decks = 2; // Number of decks in the shoe

    // Ask the player for the initial balance
    printf("Enter the initial balance: ");
    scanf("%d", &balance);

    // Calculate the minimum bet based on the initial balance
    int minBet = determineMinBet(balance);
    printf("Minimum bet based on your balance: %d\n", minBet);

    // Ask the player for the minimum bet
    printf("Enter the minimum bet (should be at least %d): ", minBet);
    scanf("%d", &baseBet);
    if (baseBet < minBet) {
        printf("The minimum bet should be at least %d. Setting base bet to %d.\n", minBet, minBet);
        baseBet = minBet;
    }

    // Ask the player for the current card count
    printf("Enter the current card count: ");
    scanf("%d", &cardCount);

    while (1) {
        int dealerCardValue, playerCard1Value, playerCard2Value;
        int isSoft = 0, canSplit = 0, playerScore = 0, numAces = 0;

        // Calculate true count
        int remainingDecks = decks - (int)strlen(seenCards) / 52;
        double trueCount = (double)cardCount / remainingDecks;

        // Calculate Kelly fraction and determine the suggested bet
        double kelly = kellyFraction(trueCount);
        int currentBet = determineBet(baseBet, kelly, balance);

        // Show the current card count, balance, and suggested bet
        printf("Current card count: %d\n", cardCount);
        printf("True count: %.2f\n", trueCount);
        printf("Suggested bet: %d\n", currentBet);
        printf("Current balance: %d\n", balance);

        // Input the player's cards
        printf("Enter the player's cards (e.g., 8K for 8 and K): ");
        scanf("%s", playerCards);

        // Input the dealer's up card
        printf("Enter the dealer's up card (e.g., K): ");
        scanf(" %c", &dealerCard);

        // Calculate the value of the player's cards
        playerCard1Value = cardValue(playerCards[0]);
        playerCard2Value = cardValue(playerCards[1]);
        if (playerCard1Value == 11 && playerCard2Value == 11) {
            // Handle the case where both cards are aces
            playerScore = 12; // Two aces should be treated as 11 + 1
            numAces = 1;
            isSoft = 1;
        } else {
            playerScore = playerCard1Value + playerCard2Value;
            if (playerCards[0] == 'A') numAces++;
            if (playerCards[1] == 'A') numAces++;
            isSoft = (numAces > 0);
        }
        if (playerCards[0] == playerCards[1]) canSplit = 1;

        // Calculate the value of the dealer's card
        dealerCardValue = cardValue(dealerCard);

        // Update the card count with the player's and dealer's cards
        cardCount += hi_lo(playerCards[0]);
        cardCount += hi_lo(playerCards[1]);
        cardCount += hi_lo(dealerCard);

        // Handle the player's hand until STAND or BUST
        while (1) {
            Action action = getAction(playerScore, dealerCardValue, isSoft, canSplit, playerCard1Value, playerCard2Value);
            if (action == STAND || playerScore > 21) {
                break;
            }

            // If HIT, add a card
            if (action == HIT) {
                printf("Player should HIT.\n");
                printf("Enter the next card for the player: ");
                char nextCard;
                scanf(" %c", &nextCard);
                int nextCardValue = cardValue(nextCard);
                updatePlayerScore(&playerScore, nextCardValue, &isSoft, &numAces);
                cardCount += hi_lo(nextCard);
            } else if (action == DOUBLE) {
                printf("Player should DOUBLE.\n");
                currentBet *= 2;
                printf("Enter the next card for the player: ");
                char nextCard;
                scanf(" %c", &nextCard);
                int nextCardValue = cardValue(nextCard);
                updatePlayerScore(&playerScore, nextCardValue, &isSoft, &numAces);
                cardCount += hi_lo(nextCard);
                break; // After DOUBLE, player must stand
            } else if (action == SPLIT) {
                // Handle split manually for now
                printf("Player should SPLIT.\n");

                // Split into two hands
                char firstHandCard1 = playerCards[0];
                char secondHandCard1 = playerCards[1];

                // First hand
                printf("Handling first hand with card %c.\n", firstHandCard1);
                playerScore = cardValue(firstHandCard1);
                isSoft = (firstHandCard1 == 'A') ? 1 : 0;
                numAces = (firstHandCard1 == 'A') ? 1 : 0;

                while (1) {
                    printf("Enter the next card for the first hand: ");
                    char nextCard;
                    scanf(" %c", &nextCard);
                    int nextCardValue = cardValue(nextCard);
                    updatePlayerScore(&playerScore, nextCardValue, &isSoft, &numAces);
                    cardCount += hi_lo(nextCard);
                    action = getAction(playerScore, dealerCardValue, isSoft, 0, firstHandCard1, nextCard);
                    if (action == STAND || playerScore > 21) {
                        break;
                    }
                    if (action == DOUBLE) {
                        printf("Player should DOUBLE.\n");
                        currentBet *= 2;
                        printf("Enter the next card for the first hand: ");
                        scanf(" %c", &nextCard);
                        nextCardValue = cardValue(nextCard);
                        updatePlayerScore(&playerScore, nextCardValue, &isSoft, &numAces);
                        cardCount += hi_lo(nextCard);
                        break; // After DOUBLE, player must stand
                    }
                }
                printf("First hand: Player stands with score %d.\n", playerScore);

                // Second hand
                printf("Handling second hand with card %c.\n", secondHandCard1);
                playerScore = cardValue(secondHandCard1);
                isSoft = (secondHandCard1 == 'A') ? 1 : 0;
                numAces = (secondHandCard1 == 'A') ? 1 : 0;

                while (1) {
                    printf("Enter the next card for the second hand: ");
                    char nextCard;
                    scanf(" %c", &nextCard);
                    int nextCardValue = cardValue(nextCard);
                    updatePlayerScore(&playerScore, nextCardValue, &isSoft, &numAces);
                    cardCount += hi_lo(nextCard);
                    action = getAction(playerScore, dealerCardValue, isSoft, 0, secondHandCard1, nextCard);
                    if (action == STAND || playerScore > 21) {
                        break;
                    }
                    if (action == DOUBLE) {
                        printf("Player should DOUBLE.\n");
                        currentBet *= 2;
                        printf("Enter the next card for the second hand: ");
                        scanf(" %c", &nextCard);
                        nextCardValue = cardValue(nextCard);
                        updatePlayerScore(&playerScore, nextCardValue, &isSoft, &numAces);
                        cardCount += hi_lo(nextCard);
                        break; // After DOUBLE, player must stand
                    }
                }
                printf("Second hand: Player stands with score %d.\n", playerScore);
                break; // Exit after handling split
            }
        }

        // Output the result of the player's hand
        if (playerScore > 21) {
            printf("Player busts with score %d.\n", playerScore);
        } else {
            printf("Player stands with score %d.\n", playerScore);
        }

        // Input the cards that have been played
        printf("Enter the cards that have been played (e.g., 23456QJT): ");
        scanf("%s", seenCards);

        // Update the card count with the played cards
        for (int i = 0; i < strlen(seenCards); i++) {
            cardCount += hi_lo(seenCards[i]);
        }

        // Update the balance based on the result of the hand
        char outcome;
        printf("Result of the hand (w for win, l for loss, p for push): ");
        scanf(" %c", &outcome);
        if (outcome == 'w' || outcome == 'W') {
            balance += currentBet;
        } else if (outcome == 'l' || outcome == 'L') {
            balance -= currentBet;
        }

        // Show the updated card count and balance
        printf("Updated card count: %d\n", cardCount);
        printf("Updated balance: %d\n", balance);

        // Ask if the player wants to play another hand
        char playAgain;
        if (balance > 0) {
            printf("Do you want to play another hand? (y/n): ");
            scanf(" %c", &playAgain);
            if (playAgain != 'y' && playAgain != 'Y') {
                break;
            }
        } else {
            printf("Balance depleted! You cannot continue playing.\n");
            break;
        }
    }

    return 0;
}