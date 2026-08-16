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

    // give player the loan money

    gamestate -> players[playerIndex].cash = gamestate -> players[playerIndex].cash + amount;

    // Record the loan details on the player
    gamestate -> players[playerIndex].hasLoan = 1;
    gamestate -> players[playerIndex].loanAmount = amount;
    gamestate -> players[playerIndex].loanInterestRate = gamestate->economy.baseInterestRate;
    gamestate -> players[playerIndex].loanRoundsLeft = LOAN_DURATION_ROUNDS;

    /* Lock up only the squares or properties told to lock by the player(players.c),
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

    // If this player doesn't have a loan.
    if (gamestate -> players[playerIndex].hasLoan == 0){
        return;
    }

    // Capping the repayment
    int currentCash = gamestate -> players[playerIndex].cash;
    int currentLoanAmount = gamestate -> players[playerIndex].loanAmount;

    if (amount > currentCash) {
        amount = currentCash;
    }
    if (amount > currentLoanAmount) {
        amount = currentLoanAmount;
    }

    // Deduct the (capped) repayment from the player cash & reduce the loan balance.
    gamestate -> players[playerIndex].cash = currentCash - amount;
    gamestate -> players[playerIndex].loanAmount = currentLoanAmount - amount;

    // Only if the loan is fully paid off, the collateral will be unlocked
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

// Sells a property back to the Bank for its current market value.
void sellProperty(GameState *gamestate, int playerIndex, int squareIndex) {
    Square *square = &gamestate -> board[squareIndex];
    Player *player = &gamestate -> players[playerIndex];

    player -> cash = player -> cash + square -> currentValue;

    square -> ownerId = -1;
    square -> numHouses = 0;
    square -> hasHotel = 0;
    square -> isMortgaged = 0;
    square -> isLoanLocked = 0;
    square -> insurancePolicy = INSURANCE_NONE;
    square -> insuranceRoundsLeft = 0;

    printf("%s sold %s for LKR %d.\n", player -> name, square -> name, square -> currentValue);
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

            // Calculating how much interest this player owes this round.
            int interest = (player -> loanAmount * player -> loanInterestRate) / 100;

            // Add the interest onto the loan balance.
            player -> loanAmount = player -> loanAmount + interest;

            // One less round left before the loan is due.
            player -> loanRoundsLeft = player -> loanRoundsLeft - 1;

            // No rounds left, loan is due now: foreclose (Rule-LK 6/7).
            if (player -> loanRoundsLeft <= 0) {

                printf("%s has defaulted.\nCollateral has been foreclosed.\nOutstanding debt cleared.\n", player -> name);

                /* Demolish buildings, cancel insurance, then auction off
                every square pledged as collateral (Rule-LK 6, Section 2.6). */
                int j = 0;
                while (j < BOARD_SIZE) {
                    if (player -> loanCollateral[j] == 1) {
                        gamestate -> board[j].numHouses = 0;
                        gamestate -> board[j].hasHotel = 0;
                        gamestate -> board[j].insurancePolicy = INSURANCE_NONE;
                        gamestate -> board[j].insuranceRoundsLeft = 0;
                        gamestate -> board[j].isMortgaged = 0;
                        gamestate -> board[j].ownerId = -1;
                        gamestate -> board[j].isLoanLocked = 0;
                        player -> loanCollateral[j] = 0;
                        runAuction(gamestate, j); // game.c
                    }
                    j = j + 1;
                }

                // The debt is settled, so wipe out the loan.
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

    // Section 1.1.2: railway stations cannot be insured.
    if (square -> type == SQUARE_RAILWAY) {
        return;
    }

    // Work out the premium percentage for the chosen policy.
    int premiumPercentage = 0;
    if (policy == INSURANCE_BASIC) {
        premiumPercentage = 5;
    } else if (policy == INSURANCE_COMPREHENSIVE) {
        premiumPercentage = 10;
    } else {
        premiumPercentage = 15;
    }

    /* Calculate how much that premium actually costs, after any active
    discount due to events (Insurance Discount card / Insurance Regulation). */
    int premium = (square -> currentValue * premiumPercentage) / 100;
    premium = premium - (premium * gamestate -> economy.insurancePremiumDiscountPercentage) / 100;

    // Only buy the policy if the player has enough cash for it
    int canAfford = 0;
    if (player -> cash >= premium) {
        canAfford = 1;
    }

    if (canAfford == 1) {
        player -> cash = player -> cash - premium;
        square -> insurancePolicy = policy;
        square -> insuranceRoundsLeft = 20;

        char *policyName = "Basic Property Insurance";
        if (policy == INSURANCE_COMPREHENSIVE) {
            policyName = "Comprehensive Insurance";
        } else if (policy == INSURANCE_BUSINESS_INTERRUPTION) {
            policyName = "Business Interruption Insurance";
        }
        printf("%s purchased.\nProperty : %s\nPremium : LKR %d.\n", policyName, square -> name, premium);
    }
}

//----paying rent---//
void payRent(GameState *gamestate, int playerIndex, int squareIndex) {
    Square *square = &gamestate -> board[squareIndex];
    if (square -> ownerId == -1 || square -> isMortgaged){
        return;
     } //No rent due

    // Rule-LK 11: A damaged building can't collect rent until repaired
    if (square -> isDamaged == 1) {
        return;
    }

    int rent = square -> currentRent; // assume already recalculated with condition/booms/etc

    // Rule-LK 26 / Table 3: building condition reduces collectable rent calculated here. 
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
         rent = payer -> cash; // Could be gone bankrupt after this
    }
    payer -> cash = payer -> cash - rent;
    owner -> cash = owner -> cash + rent;

    // Printing the output .
    printf("%s landed on %s.\nRent Paid : LKR %d.\nOwner : %s.\n", payer -> name, square -> name, rent, owner -> name);
}

// Railway rent depends on how many stations the same owner has
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

// Rent is 4x dice value for one utility owned, 10 times for both
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

/* Rule 11: collects Income Tax immediately when landed on.
15% of cash on hand, not a flat amount. */
void collectTax(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];
    int tax = (player -> cash * gamestate -> economy.currentIncomeTaxPercentage) / 100;

    if (tax > player -> cash) {
        tax = player -> cash;
    }

    player -> cash = player -> cash - tax;
    printf("%s paid Income Tax of LKR %d.\n", player -> name, tax);
}

/* Community Development Fund: 10% of the player's total Property-type value 
(not railways/utilities/buildings). */
void applyCommunityDevelopmentFundTax(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    int totalPropertyValue = 0;
    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];
        if (square -> ownerId == playerIndex && square -> type == SQUARE_PROPERTY) {
            totalPropertyValue = totalPropertyValue + square -> currentValue;
        }
        i = i + 1;
    }

    int tax = (totalPropertyValue * gamestate -> economy.communityFundTaxPercentage) / 100;
    if (tax > player -> cash) {
        tax = player -> cash;
    }

    player -> cash = player -> cash - tax;
    printf("%s paid Community Development Fund tax of LKR %d.\n", player -> name, tax);
}

int calculateNetWorth(GameState *gamestate, int playerIndex) {

    Player *player = &gamestate -> players[playerIndex];
    int netWorth = player -> cash;

    //Add up the value of every property this player owns
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

    // Subtract any outstanding loan balance.
    if (player -> hasLoan) {
        netWorth = netWorth - player -> loanAmount;
    }

    return netWorth;
}

/*Rule 14: declares a player bankrupt once their liabilities exceed their
assets (net worth drops to 0 or below), Called once per round. */
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

                /* Rule 14: buildings removed, insurance cancelled, then every
                owned square is auctioned off. */
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
                        runAuction(gamestate, j); // game.c
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

// Rule-LK 15/16: every 5 rounds past age 50, take off 1% value of the property, up to 30%
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
            if (square -> depreciationPercentage < DEPRECIATION_MAX_PERCENTAGE) {
                isUnderCap = 1;
            }

            if (isOldEnough == 1 && isFiveRoundMark == 1 && isUnderCap == 1) {
                square -> depreciationPercentage = square -> depreciationPercentage + 1;
                square -> currentValue = square -> currentValue - (square -> currentValue * 1) / 100;
                printf("Property\n%s\nhas depreciated by %d%%.\nCurrent Value\nLKR %d.\n", square -> name, square -> depreciationPercentage, square -> currentValue);
            }
        }

        i = i + 1;
    }
}

/* Rule-LK 25/26: developed properties lose 2% building condition every
round; payRent() already reads buildingCondition to cut rent as it drops */
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

/* Rule-LK 9: counts down every insured square's remaining rounds, warns
close to expiry, and cancels the policy once it reaches 0. */
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

// While Luxury Property Tax is active, hotel owners pay 25% of value each round.
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

/* Rule-LK 11: retries repair each round once the owner can afford it.
Political Rally closures (closedRoundsLeft > 0) are skipped, not repaired. */
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

/* Rule-LK 33: ticks down the 30-round cooldown on every property group so
it can't be picked again for a boom/decline until the cooldown clears. */
void decrementGroupCooldowns(GameState *gamestate) {

    int group = 1;
    while (group <= 8) {
        if (gamestate -> economy.groupCooldownRounds[group] > 0) {
            gamestate -> economy.groupCooldownRounds[group] = gamestate -> economy.groupCooldownRounds[group] - 1;
        }
        group = group + 1;
    }
}

/* Rule-LK 27: restores condition to 100%. Cost is 5% of house cost (or 8%
of hotel cost), scaled by maintenanceCostPercentage. */
void doMaintenance(GameState *gamestate, int playerIndex, int squareIndex) {
    Square *square = &gamestate -> board[squareIndex];
    Player *player = &gamestate -> players[playerIndex];

    int baseCost = 0;
    if (square -> hasHotel == 1) {
        baseCost = (square -> hotelCost * 8) / 100;
    } else {
        baseCost = (square -> houseCost * 5) / 100;
    }

    int cost = (baseCost * square -> maintenanceCostPercentage) / 100;
    if (cost > player -> cash) {
        cost = player -> cash;
    }

    player -> cash = player -> cash - cost;
    square -> buildingCondition = 100;
    square -> roundsSinceMaintenance = 0;
    printf("%s performed maintenance on %s.\nMaintenance Cost : LKR %d.\n", player -> name, square -> name, cost);
}

/* Rule-LK 28: 20+ rounds without maintenance causes one-off structural
damage - value -15%, rent -25%, future maintenance cost +50%. */
void checkMaintenanceNeglect(GameState *gamestate) {

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isDeveloped = 0;
        if (square -> numHouses > 0 || square -> hasHotel == 1) {
            isDeveloped = 1;
        }

        if (isDeveloped == 1) {
            square -> roundsSinceMaintenance = square -> roundsSinceMaintenance + 1;

            if (square -> roundsSinceMaintenance > MAINTENANCE_NEGLECT_LIMIT && square -> hasStructuralDamage == 0) {
                square -> currentValue = (square -> currentValue * 85) / 100;
                square -> currentRent = (square -> currentRent * 75) / 100;
                square -> maintenanceCostPercentage = (square -> maintenanceCostPercentage * 150) / 100;
                square -> hasStructuralDamage = 1;
                printf("%s has suffered structural damage from neglect.\n", square -> name);
            }
        }

        i = i + 1;
    }
}

/* Rule-LK 29: restores value, rent, and condition on a structurally
damaged building. Cost is 25% of the building's replacement cost. */
void renovateStructuralDamage(GameState *gamestate, int playerIndex, int squareIndex) {
    Square *square = &gamestate -> board[squareIndex];
    Player *player = &gamestate -> players[playerIndex];

    int replacementCost = square -> houseCost;
    if (square -> hasHotel == 1) {
        replacementCost = square -> hotelCost;
    }

    int cost = (replacementCost * 25) / 100;
    if (cost > player -> cash) {
        cost = player -> cash;
    }

    player -> cash = player -> cash - cost;
    square -> currentValue = (square -> currentValue * 100) / 85;
    square -> currentRent = (square -> currentRent * 100) / 75;
    square -> buildingCondition = 100;
    square -> hasStructuralDamage = 0;
    square -> roundsSinceMaintenance = 0;
    square -> maintenanceCostPercentage = 100;
    printf("%s renovated the structural damage on %s.\nRenovation Cost : LKR %d.\n", player -> name, square -> name, cost);
}

/* Rule-LK 17: restores depreciation, increases rental, resets property age.
Cost is 10% of the property's current market value. */
void renovateDepreciation(GameState *gamestate, int playerIndex, int squareIndex) {
    Square *square = &gamestate -> board[squareIndex];
    Player *player = &gamestate -> players[playerIndex];

    int cost = (square -> currentValue * 10) / 100;
    if (cost > player -> cash) {
        cost = player -> cash;
    }

    player -> cash = player -> cash - cost;
    square -> currentValue = (square -> currentValue * 100) / (100 - square -> depreciationPercentage);
    square -> currentRent = (square -> currentRent * 100) / (100 - square -> depreciationPercentage);
    square -> depreciationPercentage = 0;
    square -> propertyAge = 0;
    printf("%s renovated %s.\nRenovation Cost : LKR %d.\n", player -> name, square -> name, cost);
}

// Rule-LK 5: resets the loan's remaining duration back to 20 rounds.
void extendLoan(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];
    player -> loanRoundsLeft = LOAN_DURATION_ROUNDS;
    printf("%s extended their loan period.\nNew Duration : %d Rounds.\n", player -> name, LOAN_DURATION_ROUNDS);
}

/* Rule-LK 5: borrows additional cash on top of an existing loan, pledging
any remaining eligible collateral the same way takeLoan() does. */
void increaseLoan(GameState *gamestate, int playerIndex, int amount) {
    Player *player = &gamestate -> players[playerIndex];

    player -> cash = player -> cash + amount;
    player -> loanAmount = player -> loanAmount + amount;

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];
        if (square -> ownerId == playerIndex && square -> isMortgaged == 0 && square -> isLoanLocked == 0) {
            square -> isLoanLocked = 1;
            player -> loanCollateral[i] = 1;
        }
        i = i + 1;
    }

    printf("%s increased their loan by LKR %d.\nNew Loan Balance : LKR %d.\n", player -> name, amount, player -> loanAmount);
}

/* Anti-Speculation Act (Rule-LK 24): a property still undeveloped once its
5 round deadline runs out is auctioned off. */
void enforceDevelopmentDeadlines(GameState *gamestate) {

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        if (square -> developmentDeadlineRounds > 0) {
            square -> developmentDeadlineRounds = square -> developmentDeadlineRounds - 1;

            int isStillUndeveloped = 0;
            if (square -> numHouses == 0 && square -> hasHotel == 0) {
                isStillUndeveloped = 1;
            }

            /* assumed: on the last round before seizure, the owner sells the
            property back for its current value instead of losing it for
            nothing, since the assignment gives no rule for this case. */
            if (square -> developmentDeadlineRounds == 1 && isStillUndeveloped == 1) {
                int sellerIndex = square -> ownerId;
                sellProperty(gamestate, sellerIndex, i); // sells before seizure
            } else if (square -> developmentDeadlineRounds == 0 && isStillUndeveloped == 1) {
                printf("%s was seized for failing to develop it in time.\n", square -> name);
                square -> ownerId = -1;
                runAuction(gamestate, i); // game.c
            }
        }

        i = i + 1;
    }
}

/* Calls all of finance.c's per-round update functions in order, so game.c
only needs to call this one function once each round. */
void runFinanceUpdates(GameState *gamestate) {
    depreciateProperties(gamestate);
    decayBuildingCondition(gamestate);
    countdownInsurance(gamestate);
    applyRecurringLuxuryTax(gamestate);
    decrementGroupCooldowns(gamestate);
    retryDamageRepairs(gamestate);
    checkMaintenanceNeglect(gamestate);
    enforceDevelopmentDeadlines(gamestate);
}
