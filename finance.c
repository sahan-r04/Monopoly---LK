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
    gamestate -> players[playerIndex].loanAmount = amount;
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

// calling once at the end of every complete round for every player with a loan.
void applyLoanInterest(GameState *gamestate) {

    int i = 0;
    while (i < NUM_PLAYERS) {

        Player *player = &gamestate -> players[i];

        int playerHasLoan = 0;
        if (player -> hasLoan == 1) {
            playerHasLoan = 1;
        }

        if (playerHasLoan == 1) {

            // 1) Calculating how much interest this player owes this round.
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
//---- Buying and renewing insurance policy on single property. ----//
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

    // 2) Calculate how much that premium actually costs, after any active
    //    discount (Insurance Discount card / Insurance Regulation).
    int premium = (square -> currentValue * premiumPercentage) / 100;
    premium = premium - (premium * gamestate -> economy.insurancePremiumDiscountPercent) / 100;

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

void payRent(GameState *gamestate, int playerIndex, int squareIndex) {
    Square *square = &gamestate -> board[squareIndex];
    if (square -> ownerId == -1 || square -> isMortgaged){
        return;
     } // /No rent due

    // Rule-LK 11: a damaged building can't collect rent until repaired.
    if (square -> isDamaged == 1) {
        return;
    }

    int rent = square -> currentRent; // assume already recalculated with condition/booms/etc.

    // Rule-LK 26 / Table 3: building condition reduces collectable rent -
    // applied live here rather than baked into currentRent permanently.
    int isDeveloped = 0;
    if (square -> numHouses > 0 || square -> hasHotel == 1) {
        isDeveloped = 1;
    }

    if (isDeveloped == 1) {
        if (square -> buildingCondition < 25) {
            rent = 0;
        } else if (square -> buildingCondition < 50) {
            rent = (rent * 50) / 100;
        } else if (square -> buildingCondition < 75) {
            rent = (rent * 75) / 100;
        } else if (square -> buildingCondition < 90) {
            rent = (rent * 90) / 100;
        }
    }

    Player *payer = &gamestate -> players[playerIndex];
    Player *owner = &gamestate -> players[square -> ownerId];

    if (rent > payer -> cash) {
         rent = payer -> cash; // Could be gone bankrupt after this.
    }
    payer -> cash = payer -> cash - rent;
    owner -> cash = owner -> cash + rent;
}

// Table 2: railway rent depends on how many stations the same owner has.
int calculateRailwayRent(GameState *gamestate, int squareIndex) {
    int ownerId = gamestate -> board[squareIndex].ownerId;

    if (ownerId == -1) {
        return 0;
    }

    int stationsOwned = 0;
    int i = 0;
    while (i < BOARD_SIZE) {
        if (gamestate -> board[i].type == SQUARE_RAILWAY && gamestate -> board[i].ownerId == ownerId) {
            stationsOwned = stationsOwned + 1;
        }
        i = i + 1;
    }

    int rent = 250;
    if (stationsOwned == 2) {
        rent = 500;
    } else if (stationsOwned == 3) {
        rent = 1000;
    } else if (stationsOwned == 4) {
        rent = 2000;
    }

    return rent;
}

// Section 1.1.3: utility rent is dice-based - 4x dice value for one utility
// owned, 10x for both.
int calculateUtilityRent(GameState *gamestate, int squareIndex, int diceTotal) {
    int ownerId = gamestate -> board[squareIndex].ownerId;

    if (ownerId == -1) {
        return 0;
    }

    int utilitiesOwned = 0;
    int i = 0;
    while (i < BOARD_SIZE) {
        if (gamestate -> board[i].type == SQUARE_UTILITY && gamestate -> board[i].ownerId == ownerId) {
            utilitiesOwned = utilitiesOwned + 1;
        }
        i = i + 1;
    }

    int multiplier = 4;
    if (utilitiesOwned == 2) {
        multiplier = 10;
    }

    return diceTotal * multiplier;
}

// Rule 11: collects Income Tax immediately when landed on.
void collectTax(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];
    int tax = gamestate -> economy.currentIncomeTaxAmount;

    if (tax > player -> cash) {
        tax = player -> cash;
    }

    player -> cash = player -> cash - tax;
    printf("%s paid Income Tax of LKR %d.\n", player -> name, tax);
}

int calculateNetWorth(GameState *gamestate, int playerIndex) {

    Player *player = &gamestate -> players[playerIndex];
    int netWorth = player -> cash;

    // 1) Add up the value of every property this player owns.
    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isOwnedByThisPlayer = 0;
        if (square -> ownerId == playerIndex) {
            isOwnedByThisPlayer = 1;
        }

        if (isOwnedByThisPlayer == 1) {
            netWorth = netWorth + square -> currentValue;

            if (square -> isMortgaged) {
                netWorth = netWorth - square -> mortgageValue;
            }
        }

        i = i + 1;
    }

    // 2) Subtract any outstanding loan balance.
    if (player -> hasLoan) {
        netWorth = netWorth - player -> loanAmount;
    }

    return netWorth;
}

// Rule 14: declares a player bankrupt once their liabilities exceed their
// assets (net worth drops to 0 or below). Called once per round.
void checkBankruptcy(GameState *gamestate) {

    int i = 0;
    while (i < NUM_PLAYERS) {
        Player *player = &gamestate -> players[i];

        int alreadyBankrupt = 0;
        if (player -> isBankrupt == 1) {
            alreadyBankrupt = 1;
        }

        if (alreadyBankrupt == 0) {

            int netWorth = calculateNetWorth(gamestate, i);

            int isNowBankrupt = 0;
            if (netWorth <= 0) {
                isNowBankrupt = 1;
            }

            if (isNowBankrupt == 1) {
                player -> isBankrupt = 1;

                // Rule 14: buildings removed, insurance cancelled, and every
                // owned square returns to the Bank.
                int j = 0;
                while (j < BOARD_SIZE) {
                    Square *square = &gamestate -> board[j];
                    if (square -> ownerId == i) {
                        square -> numHouses = 0;
                        square -> hasHotel = 0;
                        square -> insurancePolicy = INSURANCE_NONE;
                        square -> insuranceRoundsLeft = 0;
                        square -> isMortgaged = 0;
                        square -> isLoanLocked = 0;
                        square -> ownerId = -1;
                    }
                    j = j + 1;
                }

                // Rule 14: outstanding debt is cleared along with the loan.
                player -> hasLoan = 0;
                player -> loanAmount = 0;
                player -> loanInterestRate = 0;
                player -> loanRoundsLeft = 0;

                printf("%s has been declared bankrupt.\nRemaining assets transferred to the Bank.\n", player -> name);
            }
        }

        i = i + 1;
    }
}

// Rule-LK 15/16: ages every property by one round, then knocks another 1%
// off currentValue each time a new 5-round threshold past age 50 is
// crossed, capped at 30% total.
void depreciateProperties(GameState *gamestate) {

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isProperty = 0;
        if (square -> type == SQUARE_PROPERTY) {
            isProperty = 1;
        }

        if (isProperty == 1) {
            square -> propertyAge = square -> propertyAge + 1;

            int isOldEnough = 0;
            if (square -> propertyAge > DEPRECIATION_AGE_LIMIT) {
                isOldEnough = 1;
            }

            int isFiveRoundMark = 0;
            if (square -> propertyAge % 5 == 0) {
                isFiveRoundMark = 1;
            }

            int isUnderCap = 0;
            if (square -> depreciationPercent < DEPRECIATION_MAX_PERCENTAGE) {
                isUnderCap = 1;
            }

            if (isOldEnough == 1 && isFiveRoundMark == 1 && isUnderCap == 1) {
                square -> depreciationPercent = square -> depreciationPercent + 1;
                square -> currentValue = square -> currentValue - (square -> currentValue * 1) / 100;
            }
        }

        i = i + 1;
    }
}

// Rule-LK 25/26: developed properties lose 2% building condition every
// round; payRent() already reads buildingCondition to cut rent as it drops.
void decayBuildingCondition(GameState *gamestate) {

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isDeveloped = 0;
        if (square -> numHouses > 0 || square -> hasHotel == 1) {
            isDeveloped = 1;
        }

        if (isDeveloped == 1) {
            square -> buildingCondition = square -> buildingCondition - BUILDING_DECAY_PERCENTAGE_PER_ROUND;
            if (square -> buildingCondition < 0) {
                square -> buildingCondition = 0;
            }
        }

        i = i + 1;
    }
}

// Rule-LK 9: counts down every insured square's remaining rounds, warns
// close to expiry, and cancels the policy once it reaches 0.
void countdownInsurance(GameState *gamestate) {

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isInsured = 0;
        if (square -> insurancePolicy != INSURANCE_NONE) {
            isInsured = 1;
        }

        if (isInsured == 1) {
            square -> insuranceRoundsLeft = square -> insuranceRoundsLeft - 1;

            if (square -> insuranceRoundsLeft == INSURANCE_WARNING_ROUNDS) {
                printf("Insurance on %s expires in %d rounds.\n", square -> name, INSURANCE_WARNING_ROUNDS);
            }

            if (square -> insuranceRoundsLeft <= 0) {
                square -> insurancePolicy = INSURANCE_NONE;
                square -> insuranceRoundsLeft = 0;
            }
        }

        i = i + 1;
    }
}

// Replaces governmentRegulationChange()'s old one-off Luxury Property Tax
// charge - while that regulation stays active, hotel owners are charged
// 25% of the hotel square's value again every round.
void applyRecurringLuxuryTax(GameState *gamestate) {

    int luxuryTaxActive = 0;
    if (gamestate -> economy.governmentRegulation.isActive == 1 && gamestate -> economy.governmentRegulation.cardId == 3) {
        luxuryTaxActive = 1;
    }

    if (luxuryTaxActive == 1) {
        int i = 0;
        while (i < BOARD_SIZE) {
            Square *square = &gamestate -> board[i];

            if (square -> hasHotel == 1 && square -> ownerId != -1) {
                Player *owner = &gamestate -> players[square -> ownerId];
                int tax = (square -> currentValue * 25) / 100;

                if (tax > owner -> cash) {
                    tax = owner -> cash;
                }
                owner -> cash = owner -> cash - tax;
            }

            i = i + 1;
        }
    }
}

// Rule-LK 11: retries repairs each round for a disaster-damaged property
// once its owner can finally afford the cost. Political Rally closures are
// skipped (closedRoundsLeft > 0) - those clear on their own via
// board.c's reopenClosedSquares() instead of being repaired.
void retryDamageRepairs(GameState *gamestate) {

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isGenuinelyDamaged = 0;
        if (square -> isDamaged == 1 && square -> closedRoundsLeft == 0 && square -> ownerId != -1) {
            isGenuinelyDamaged = 1;
        }

        if (isGenuinelyDamaged == 1) {
            int repairCost = (square -> currentValue * 10) / 100;
            Player *owner = &gamestate -> players[square -> ownerId];

            if (owner -> cash >= repairCost) {
                owner -> cash = owner -> cash - repairCost;
                square -> isDamaged = 0;
                printf("%s's %s has been repaired.\nRepair Cost Paid : LKR %d.\n", owner -> name, square -> name, repairCost);
            }
        }

        i = i + 1;
    }
}

// Rule-LK 33: ticks down the 30-round cooldown on every property group so
// it can't be picked again for a boom/decline until the cooldown clears.
void decrementGroupCooldowns(GameState *gamestate) {

    int group = 1;
    while (group <= 8) {
        if (gamestate -> economy.groupCooldownRounds[group] > 0) {
            gamestate -> economy.groupCooldownRounds[group] = gamestate -> economy.groupCooldownRounds[group] - 1;
        }
        group = group + 1;
    }
}

// Single call site for game.c: runs every round-tick system in finance.c
// in sequence, once per completed round.
void applyRoundTickEffects(GameState *gamestate) {
    depreciateProperties(gamestate);
    decayBuildingCondition(gamestate);
    countdownInsurance(gamestate);
    applyRecurringLuxuryTax(gamestate);
    decrementGroupCooldowns(gamestate);
    retryDamageRepairs(gamestate);
}
