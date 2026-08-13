#include <stdio.h>
#include <stdlib.h>
#include "types.h"

/*   Checking for eligibility of monopoly play
for a given player and property group - Rule 8  */
int monopolyPlay(GameState *gamestate, int playerIndex, PropertyGroup group) {

    int hasMonopolyPlay = 1;

    /* Go through every square on the board and check whether that belongs to
       this property group */
    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isInGroup = 0;
        if (square -> group == group) {
            isInGroup = 1;
        }

        if (isInGroup == 1) {

            int ownedByThisPlayer = 0;
            if (square -> ownerId == playerIndex) {
                ownedByThisPlayer = 1;
            }

            if (ownedByThisPlayer == 0) {
                // This square in the group belongs to someone else (or nobody) -
                // so the player doesn't have a monopoly on this group.
                hasMonopolyPlay = 0;
            }
        }

        i = i + 1;
    }

    return hasMonopolyPlay;
}

int decideToPurchase(GameState *gamestate, int playerIndex, int squareIndex) {
    Player *player = &gamestate -> players[playerIndex];
    Square *square = &gamestate -> board[squareIndex];

    // Anti-Speculation Act (Rule-LK 24): while active, a player already
    // holding 3 undeveloped properties may not buy another.
    if (gamestate -> economy.antiSpeculationActive == 1) {
        int undevelopedCount = 0;
        int i = 0;
        while (i < BOARD_SIZE) {
            Square *ownedSquare = &gamestate -> board[i];
            if (ownedSquare -> ownerId == playerIndex && ownedSquare -> type == SQUARE_PROPERTY && ownedSquare -> numHouses == 0 && ownedSquare -> hasHotel == 0) {
                undevelopedCount = undevelopedCount + 1;
            }
            i = i + 1;
        }
        if (undevelopedCount >= 3) {
            return 0;
        }
    }

    switch (player -> strategy) {
        case STRATEGY_AGGRESSIVE: //always pays if got funds, at least one future rent
            return ((player -> cash) - (square -> purchasePrice)) >= square -> baseRent;

        case STRATEGY_CONSERVATIVE: //pays only if 50% of the current cash remains after paying
            return ((player -> cash) - (square -> purchasePrice)) >= ((player -> cash) / 2);

        case STRATEGY_RISK_TAKER: //buys every property whenever legally possible 
            return (square -> purchasePrice <= player -> cash);

        case STRATEGY_OPPORTUNISTIC: //Only buys if the property's future value > building cost)
            return (square -> currentValue > square -> purchasePrice) && (player -> cash >= square -> purchasePrice);
    }
    return 0;  
}

int decideAuctionBid(GameState *gamestate, int playerIndex, int squareIndex, int currentBid) {

    Player *player = &gamestate -> players[playerIndex];
    Square *square = &gamestate -> board[squareIndex];

    //The minimum increment above the current one (Rule-LK 20).
    int nextBid = currentBid + 250;

    //Calculate the highest amount this player's strategy is willing to bid
    int maxWillingToBid = 0;

    if (player -> strategy == STRATEGY_AGGRESSIVE) {
        // Bids aggressively until 120% of market value
        maxWillingToBid = ((square -> currentValue) * 120) / 100;

    } else if (player -> strategy == STRATEGY_CONSERVATIVE) {
        // Only bids below the market value
        maxWillingToBid = square -> currentValue;

    } else if (player -> strategy == STRATEGY_RISK_TAKER) {
        //Bids until cash is gone 
        maxWillingToBid = player -> cash;

    } else {
        //prefers discounted auction purchases only.
        maxWillingToBid = (((square -> currentValue) * 80) / 100);
    }

    /*The player only actually places the bid if it's within their strategy's
    limit and they can really afford it (Rule-LK 22)*/
    int wantsToBid = 0;
    if (nextBid <= maxWillingToBid && nextBid <= player -> cash) {
        wantsToBid = 1;
    }

    //-1 = player is out from the auction 
    int decision = -1;
    if (wantsToBid == 1) {
        decision = nextBid;
    }
    return decision;
}

int decideLoanAmount(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];
    int maxLoan = calculateMaxLoan(gamestate, playerIndex);

    if (player -> strategy == STRATEGY_AGGRESSIVE) {
        // Rule: "Obtains loans whenever additional funds can increase projected
        // rental income" - true only if this player owns a monopoly group with room to build (Table 6).
        int canImproveRentalIncome = 0;
        int i = 0;
        while (i < BOARD_SIZE) {
            Square *square = &gamestate -> board[i];

            int ownsThisSquare = 0;
            if (square -> ownerId == playerIndex) {
                ownsThisSquare = 1;
            }

            int hasSpaceToBuild = 0;
            if (square -> numHouses < MAX_HOUSES && square -> hasHotel == 0) {
                hasSpaceToBuild = 1;
            }

            int ownsMonopolyOnThisGroup = 0;
            if (square -> type == SQUARE_PROPERTY) {
                if (monopolyPlay(gamestate, playerIndex, square -> group) == 1) {
                    ownsMonopolyOnThisGroup = 1;
                }
            }

            if (ownsThisSquare == 1 && hasSpaceToBuild == 1 && ownsMonopolyOnThisGroup == 1) {
                canImproveRentalIncome = 1;
            }

            i = i + 1;
        }

        if (canImproveRentalIncome == 1) {
            return maxLoan / 2;
        } else {
            return 0;
        }

    } else if (player -> strategy == STRATEGY_CONSERVATIVE) {
        // Rule: "Avoids obtaining loans unless bankruptcy is imminent." ASSUMPTION -
        // "imminent" isn't defined, so cash below 5% of starting cash is the danger line.
        int bankruptcyImminent = 0;
        if (player -> cash < (STARTING_CASH * 5) / 100) {
            bankruptcyImminent = 1;
        }

        if (bankruptcyImminent == 1) {
            return maxLoan;
        } else {
            return 0;
        }

    } else if (player -> strategy == STRATEGY_RISK_TAKER) {
        // Rule: "Always borrows the maximum loan permitted."
        return maxLoan;

    } else {
        // STRATEGY_OPPORTUNISTIC - Rule: "loans only when projected return exceeds
        // borrowing cost." ASSUMPTION - return = current total rent income; cost = one round of maxLoan interest (Rule-LK 4, same formula as applyLoanInterest()).
        int projectedReturn = 0;
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].ownerId == playerIndex) {
                projectedReturn = projectedReturn + (gamestate -> board[i].currentRent);
            }
            i = i + 1;
        }

        int borrowingCost = (maxLoan * (gamestate -> economy.baseInterestRate)) / 100;

        if (projectedReturn > borrowingCost) {
            return maxLoan / 3;
        } else {
            return 0;
        }
    }
}

int decideLoanRepaymentAmount(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    if (player -> hasLoan == 0) {
        return 0;
    }

    if (player -> strategy == STRATEGY_AGGRESSIVE) {
        // Rule: "Repays loans only when excess cash exceeds twice the outstanding loan."
        if (player -> cash > (2 * (player -> loanAmount))) {
            return player -> loanAmount;
        } else {
            return 0;
        }

    } else if (player -> strategy == STRATEGY_CONSERVATIVE) {
        // Rule:"Repays immediately if sufficient funds exist" - repayLoan() in finance.c
        // already caps this at available cash, so this always tries the full amount.
        if (player -> cash > 0) {
            return player -> loanAmount;
        } else {
            return 0;
        }

    } else if (player -> strategy == STRATEGY_RISK_TAKER) {
        // Never voluntarily repays.
        return 0;

    } else {
        // STRATEGY_OPPORTUNISTIC - repays only if plenty of cash remains afterwards.
        int cashAfterRepaying = (player -> cash) - (player -> loanAmount);
        if (cashAfterRepaying >= ((player -> cash) / 2)) {
            return player -> loanAmount;
        } else {
            return 0;
        }
    }
}

int decideInsuranceTarget(GameState *gamestate, int playerIndex) {

    // Finds the first developed, uninsured property this player owns - not
    // specified per-strategy in Section 3, so every strategy uses the same target rule.
    int targetSquare = -1;
    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int ownsThisSquare = 0;
        if (square -> ownerId == playerIndex) {
            ownsThisSquare = 1;
        }

        int isUninsured = 0;
        if (square -> insurancePolicy == INSURANCE_NONE) {
            isUninsured = 1;
        }

        int isDeveloped = 0;
        if (square -> numHouses > 0 || square -> hasHotel == 1) {
            isDeveloped = 1;
        }

        if (ownsThisSquare == 1 && isUninsured == 1 && isDeveloped == 1 && targetSquare == -1) {
            targetSquare = i;
        }

        i = i + 1;
    }

    return targetSquare;
}

InsuranceType decideInsurancePolicy(GameState *gamestate, int playerIndex, int squareIndex) {
    Player *player = &gamestate -> players[playerIndex];
    Square *square = &gamestate -> board[squareIndex];

    InsuranceType chosenPolicy = INSURANCE_NONE;

    if (player -> strategy == STRATEGY_AGGRESSIVE) {
        // Rule: "Basic Insurance for houses, Comprehensive Insurance for hotels."
        if (square -> hasHotel == 1) {
            chosenPolicy = INSURANCE_COMPREHENSIVE;
        } else if (square -> numHouses > 0) {
            chosenPolicy = INSURANCE_BASIC;
        }

    } else if (player -> strategy == STRATEGY_CONSERVATIVE) {
        // Rule: "Always purchases Comprehensive Insurance for every developed property."
        chosenPolicy = INSURANCE_COMPREHENSIVE;

    } else if (player -> strategy == STRATEGY_RISK_TAKER) {
        // Rule: "Purchases insurance only after experiencing a financial loss."
        // ASSUMPTION - approximated as "only once this property is already damaged."
        if (square -> isDamaged == 1) {
            chosenPolicy = INSURANCE_BASIC;
        }

    } else {
        // STRATEGY_OPPORTUNISTIC - Rule: "Comprehensive Insurance only for high-value
        // developments." ASSUMPTION - a hotel counts as "high-value."
        if (square -> hasHotel == 1) {
            chosenPolicy = INSURANCE_COMPREHENSIVE;
        }
    }

    return chosenPolicy;
}

int decideConstruction(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    // Labour Strike / Political Unrest: no construction while suspended.
    if (gamestate -> economy.constructionSuspendedRoundsLeft > 0) {
        return -1;
    }

    // Rule 9: build evenly - always pick the square in this player's
    // holdings with the fewest houses so far.
    int chosenSquare = -1;
    int lowestHouseCount = MAX_HOUSES + 1;

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int ownsThisSquare = 0;
        if (square -> ownerId == playerIndex) {
            ownsThisSquare = 1;
        }

        int isPartOfMonopoly = 0;
        if (square -> type == SQUARE_PROPERTY) {
            if (monopolyPlay(gamestate, playerIndex, square -> group) == 1) {
                isPartOfMonopoly = 1;
            }
        }

        int hasRoomToBuild = 0;
        if (square -> numHouses < MAX_HOUSES && square -> hasHotel == 0) {
            hasRoomToBuild = 1;
        }

        // Rule: "Never develops hotels until all outstanding loans have been settled."
        int allowedToBuildHere = 1;
        if (player -> strategy == STRATEGY_CONSERVATIVE && square -> numHouses == MAX_HOUSES && player -> hasLoan == 1) {
            allowedToBuildHere = 0;
        }

        if (ownsThisSquare == 1 && isPartOfMonopoly == 1 && hasRoomToBuild == 1 && allowedToBuildHere == 1) {
            if (square -> numHouses < lowestHouseCount) {
                lowestHouseCount = square -> numHouses;
                chosenSquare = i;
            }
        }

        i = i + 1;
    }

    if (chosenSquare == -1) {
        return -1;
    }

    // STRATEGY_OPPORTUNISTIC only builds when the timing is right (delays during
    // inflation, accelerates during a Housing Subsidy regulation).
    if (player -> strategy == STRATEGY_OPPORTUNISTIC) {
        int housingSubsidyActive = 0;
        if (gamestate -> economy.governmentRegulation.isActive == 1 && gamestate -> economy.governmentRegulation.effectId == 2) {
            housingSubsidyActive = 1;
        }

        if (gamestate -> economy.inflationRate > 0 && housingSubsidyActive == 0) {
            return -1;
        }
    }
    return chosenSquare;
}

int decideMortgage(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    // ASSUMPTION - no strategy states a mortgage rule, so this treats mortgaging as
    // a last resort: cash below the same "danger line" used for Conservative's bankruptcy check.
    int cashIsCritical = 0;
    if (player -> cash < (STARTING_CASH * 5) / 100) {
        cashIsCritical = 1;
    }

    if (cashIsCritical == 0) {
        return -1;
    }

    // Pick the eligible, unmortgaged, unlocked property with the highest mortgage
    // value, so one mortgage raises as much emergency cash as possible.
    int chosenSquare = -1;
    int highestMortgageValue = -1;

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isEligible = 0;
        if (square -> ownerId == playerIndex && square -> isMortgaged == 0 && square -> isLoanLocked == 0) {
            isEligible = 1;
        }

        if (isEligible == 1) {
            if (square -> mortgageValue > highestMortgageValue) {
                highestMortgageValue = square -> mortgageValue;
                chosenSquare = i;
            }
        }

        i = i + 1;
    }

    return chosenSquare;
}