#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "types.h"

//---- Rule-LK 2 ----//
int calculateMaxLoan(GameState *gamestate, int playerIndex){

    int totalMortgageValue = 0;

    int i = 0;

    while (i < BOARD_SIZE){
        Square landedSquare = gamestate -> board[i];

        int isOwnedByThisPlayer = 0;
        if(landedSquare.ownerId == playerIndex){
            isOwnedByThisPlayer = 1;
        }

        int isMortgaged = 0;
        if(landedSquare.isMortgaged == 1){
            isMortgaged = 1;
        }

        int isLoanLocked = 0;
        if(landedSquare.isLoanLocked == 1){
            isLoanLocked = 1;
        }

        if(isOwnedByThisPlayer == 1){
            if(isMortgaged == 0){
                if(isLoanLocked == 0){
                    totalMortgageValue = totalMortgageValue + landedSquare.mortgageValue;
                }
            }
        }
        i++;
    }
    int maxLoan = (totalMortgageValue*75)/100;
    return maxLoan;
}

//---- Rule-LK 3 ----//
void takeLoan(GameState *gamestate, int playerIndex, int amount, int collateralChoice[BOARD_SIZE]){

    // 1) give player the loan money

    gamestate -> players[playerIndex].cash = gamestate -> players[playerIndex].cash + amount;

    // 2) Record the loan details on the player

    gamestate -> players[playerIndex].hasLoan = 1;
    gamestate ->players[playerIndex].loanAmount = amount;
    gamestate -> players[playerIndex].loanInterestRate = gamestate->economy.baseInterestRate;
    gamestate -> players[playerIndex].loanRoundsLeft = LOAN_DURATION_ROUNDS;

    /* 3) Lock up only the squares or properties told to lock by the player(players.c),
    according to the collateralChoice[] */

    int i = 0;
    while (i < BOARD_SIZE) {

        if (collateralChoice[i] == 1){
            gamestate -> board[i].isLoanLocked = 1;
            gamestate -> players[playerIndex].loanCollateral[i] = 1;
        }
        i = i + 1;
    }
}

//---- Rule-LK 5 ----//
void repayLoan(GameState *gamestate, int playerIndex, int amount){

    // 1) If this player doesn't have a loan.
    if (gamestate -> players[playerIndex].hasLoan == 0){
        return;
    }

    // 2) Capping the repayment.

    int currentCash = gamestate -> players[playerIndex].cash;
    int currentLoanAmount = gamestate -> players[playerIndex].loanAmount;

    if (amount > currentCash) {
        amount = currentCash;
    }
    if (amount > currentLoanAmount) {
        amount = currentLoanAmount;
    }

    // 3) Deduct the (capped) repayment from the player cash & reduce the loan balance.

    gamestate -> players[playerIndex].cash = currentCash - amount;
    gamestate -> players[playerIndex].loanAmount = currentLoanAmount - amount;

    // 4) Only if the loan is fully paid off, the collateral will be unlocked.

    if (gamestate -> players[playerIndex].loanAmount == 0) {

        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> players[playerIndex].loanCollateral[i] == 1) {
                gamestate -> board[i].isLoanLocked = 0;
                gamestate -> players[playerIndex].loanCollateral[i] = 0;
            }
            i = i + 1;
        }

        gamestate -> players[playerIndex].hasLoan = 0;
        gamestate -> players[playerIndex].loanAmount = 0;
        gamestate -> players[playerIndex].loanInterestRate = 0;
        gamestate -> players[playerIndex].loanRoundsLeft = 0;
    }
}

// called once at the end of every complete round for every player with a loan.
void applyLoanInterest(GameState *gamestate) {

    int i = 0;
    while (i < NUM_PLAYERS) {

        Player *player = &gamestate -> players[i];

        int playerHasLoan = 0;
        if (player->hasLoan == 1) {
            playerHasLoan = 1;
        }

        if (playerHasLoan == 1) {

            // 1) Work out how much interest this player owes this round.
            int interest = (player -> loanAmount * player -> loanInterestRate) / 100;

            // 2) Add the interest onto the loan balance.
            player -> loanAmount = player -> loanAmount + interest;

            // 3) One less round left before the loan is due.
            player -> loanRoundsLeft = player -> loanRoundsLeft - 1;

            // 4) If no rounds are left, the loan is due now - foreclose (Rule-LK 6/7).
            if (player -> loanRoundsLeft <= 0) {

                // 4a) Give every square pledged as collateral back to the bank.
                int j = 0;
                while (j < BOARD_SIZE) {
                    if (player -> loanCollateral[j] == 1) {
                        gamestate->board[j].ownerId = -1;
                        gamestate->board[j].isLoanLocked = 0;
                        player->loanCollateral[j] = 0;
                    }
                    j = j + 1;
                }

                // 4b) The debt is settled, so wipe out the loan.
                player -> hasLoan = 0;
                player -> loanAmount = 0;
                player -> loanInterestRate = 0;
                player -> loanRoundsLeft = 0;
            }
        }

        i = i + 1;
    }
}
//---- Buying and renewing insurance policy on one property. ----//
void getInsurance(GameState *gamestate, int playerIndex, int squareIndex, InsuranceType policy) {
    
    Square *square = &gamestate -> board[squareIndex];
    Player *player = &gamestate -> players[playerIndex];

    // 1) Work out the premium percentage for the chosen policy.
    int premiumPercentage = 0;
    if (policy == INSURANCE_BASIC) {
        premiumPercentage = 5;
    } else if (policy == INSURANCE_COMPREHENSIVE) {
        premiumPercentage = 10;
    } else {
        premiumPercentage = 15;
    }

    // 2) Calculate how much that premium actually costs.
    int premium = (square -> currentValue * premiumPercentage) / 100;

    // 3) Only buy the policy if the player has enough cash for it
    int canAfford = 0;
    if (player -> cash >= premium) {
        canAfford = 1;
    }

    if (canAfford == 1) {
        player -> cash = player -> cash - premium;
        square -> insurancePolicy = policy;
        square -> insuranceRoundsLeft = 20;
    }
}

