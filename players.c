#include <stdio.h>
#include <stdlib.h>
#include "types.h"

// Checks if a player owns every property in a group (Rule 8).
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
                /* This square in the group belongs to someone else or nobody -
                so the player doesn't have a monopoly on this group */
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

    /* Anti-Speculation Act (Rule-LK 24): While active, a player already
    holding 3 undeveloped properties may not buy another */
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
        case STRATEGY_AGGRESSIVE: //Always pays if got funds, at least one future rent
            return ((player -> cash) - (square -> purchasePrice)) >= square -> baseRent;

        case STRATEGY_CONSERVATIVE: //Pays only if 50% of the current cash remains after paying
            return ((player -> cash) - (square -> purchasePrice)) >= ((player -> cash) / 2);

        case STRATEGY_RISK_TAKER: //Buys every property whenever legally possible 
            return (square -> purchasePrice <= player -> cash);

        case STRATEGY_OPPORTUNISTIC: //Only buys if the property's future value > building cost)
            return (square -> currentValue > square -> purchasePrice) && (player -> cash >= square -> purchasePrice);
    }
    return 0;  
}

int decideAuctionBid(GameState *gamestate, int playerIndex, int squareIndex, int currentBid) {

    Player *player = &gamestate -> players[playerIndex];
    Square *square = &gamestate -> board[squareIndex];

    //The minimum increment above the current one (Rule-LK 20)
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
    limit and they can really afford it(Rule-LK 22)*/
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
        /* Rule: loan increases rental income only if a monopoly still has
        room to build (Table 6)*/
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
        /* Rule: Avoids obtaining loans unless bankruptcy is imminent. Assumed -
        "imminent" isn't defined, so cash below 5% of starting cash is the danger line. */
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
        // Rule: Always borrows the maximum loan permitted
        return maxLoan;

    } else {
        // Assumed: return = current rent income, cost = one round of maxLoan interest(Rule-LK 4)
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
        // Rule: Repays loans only when excess cash exceeds twice the outstanding loan.
        if (player -> cash > (2 * (player -> loanAmount))) {
            return player -> loanAmount;
        } else {
            return 0;
        }

    } else if (player -> strategy == STRATEGY_CONSERVATIVE) {
        // Always tries the full amount, repayLoan() caps it at available cash
        if (player -> cash > 0) {
            return player -> loanAmount;
        } else {
            return 0;
        }

    } else if (player -> strategy == STRATEGY_RISK_TAKER) {
        // Never voluntarily repays.
        return 0;

    } else {
        // Opportuunistic Trader repays only if plenty of cash remains after
        int cashAfterRepaying = (player -> cash) - (player -> loanAmount);
        if (cashAfterRepaying >= ((player -> cash) / 2)) {
            return player -> loanAmount;
        } else {
            return 0;
        }
    }
}

int decideInsuranceTarget(GameState *gamestate, int playerIndex) {

    /* Assumption:Finds the first developed, uninsured property owned. Not specified
    per-strategy, so every strategy uses this same rule. */
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

        // Railway stations cannot be insured or developed 
        int isInsurable = 1;
        if (square -> type == SQUARE_RAILWAY) {
            isInsurable = 0;
            isDeveloped = 0;
        }

        if (ownsThisSquare == 1 && isUninsured == 1 && isDeveloped == 1 && isInsurable == 1 && targetSquare == -1) {
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
        // Rule: Basic Insurance for houses, Comprehensive Insurance for hotels
        if (square -> hasHotel == 1) {
            chosenPolicy = INSURANCE_COMPREHENSIVE;
        } else if (square -> numHouses > 0) {
            chosenPolicy = INSURANCE_BASIC;
        }

    } else if (player -> strategy == STRATEGY_CONSERVATIVE) {
        // Rule: Always purchases Comprehensive Insurance for every developed property
        chosenPolicy = INSURANCE_COMPREHENSIVE;

    } else if (player -> strategy == STRATEGY_RISK_TAKER) {
        // Assumed - after a loss approximated as already damaged
        if (square -> isDamaged == 1) {
            chosenPolicy = INSURANCE_BASIC;
        }

    } else {
        // Assumed - A hotel counts as high-valued (Comprehensive only).
        if (square -> hasHotel == 1) {
            chosenPolicy = INSURANCE_COMPREHENSIVE;
        }
    }

    return chosenPolicy;
}

int decideConstruction(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    // Labour Strike /Political Unrest: no construction while suspended
    if (gamestate -> economy.constructionSuspendedRoundsLeft > 0) {
        return -1;
    }

    // Rule 9:Build evenly - Pick the owned square with fewest houses
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

        // Rule:Never develops hotels until all outstanding loans have been settled
        int allowedToBuildHere = 1;
        if (player -> strategy == STRATEGY_CONSERVATIVE && square -> numHouses == MAX_HOUSES && player -> hasLoan == 1) {
            allowedToBuildHere = 0;
        }

        // Can't build what can't be afforded.
        int buildCost = square -> houseCost;
        if (square -> numHouses == MAX_HOUSES) {
            buildCost = square -> hotelCost;
        }
        int canAfford = 0;
        if (buildCost <= player -> cash) {
            canAfford = 1;
        }

        if (ownsThisSquare == 1 && isPartOfMonopoly == 1 && hasRoomToBuild == 1 && allowedToBuildHere == 1 && canAfford == 1) {
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

    /* Opprtunistic trader only builds when the timing is favourable (delays during
    inflation, decides during a housing subsidy regulation) */
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

    /* Only mortgages a property as the last option, once cash drops below the
     same 5% of starting cash line used for Conservative Banker's bankruptcy check.*/
    int cashIsCritical = 0;
    if (player -> cash < (STARTING_CASH *5) / 100) {
        cashIsCritical = 1;
    }

    if (cashIsCritical == 0) {
        return -1;
    }

    /* Pick the eligible, unmortgaged, unlocked property with the highest mortgage
    value, so one mortgage raises as much emergency cash as possible. */
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

int decidePayOffMortgage(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    /* Assumption - Only pays off a mortgage if cash stays above the same 5% of starting cash
     safety line decideMortgage() uses */
    int chosenSquare = -1;
    int lowestMortgageValue = -1;

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isEligible = 0;
        if (square -> ownerId == playerIndex && square -> isMortgaged == 1) {
            isEligible = 1;
        }

        if (isEligible == 1) {
            int staysSafeAfterPaying = 0;
            if (player -> cash - square -> mortgageValue >= (STARTING_CASH * 5) / 100) {
                staysSafeAfterPaying = 1;
            }

            /* Pick the cheapest eligible mortgage to pay off first, so limited
            spare cash reclaims as many rent-earning properties as possible.*/
            if (staysSafeAfterPaying ==1) {
                if (lowestMortgageValue == -1 || square -> mortgageValue < lowestMortgageValue) {
                    lowestMortgageValue = square -> mortgageValue;
                    chosenSquare = i;
                }
            }
        }

        i = i + 1;
    }

    return chosenSquare;
}

/* Fixes the worst-condition building the player can afford. Assmed - Risk Taker
  waits until it's about to stop earning rent (25% - below buildings will be closed) 
  instead of fixing it early.*/
int decideToMaintain(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    int conditionThreshold = 100;
    if (player -> strategy == STRATEGY_RISK_TAKER) {
        conditionThreshold = 25;
    }

    int chosenSquare = -1;
    int lowestCondition = 101;

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isDeveloped = 0;
        if (square -> numHouses > 0 || square -> hasHotel == 1) {
            isDeveloped = 1;
        }

        int isEligible = 0;
        if (square -> ownerId == playerIndex && isDeveloped == 1 && square -> buildingCondition < conditionThreshold) {
            isEligible = 1;
        }

        if (isEligible == 1) {
            int baseCost = (square -> houseCost * 5) / 100;
            if (square -> hasHotel == 1) {
                baseCost = (square -> hotelCost* 8) / 100;
            }
            int cost = (baseCost * square -> maintenanceCostPercentage) / 100;

            if (cost <= player -> cash && square -> buildingCondition < lowestCondition) {
                lowestCondition = square -> buildingCondition;
                chosenSquare = i;
            }
        }

        i = i + 1;
    }

    return chosenSquare;
}

/* Rule-LK 29: picks the first affordable structurally damaged property to
renovate. Assumed: every strategy behaves the same */
int decideRenovateStructuralDamage(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isEligible = 0;
        if (square -> ownerId == playerIndex && square -> hasStructuralDamage == 1) {
            isEligible = 1;
        }

        if (isEligible == 1) {
            int replacementCost = square -> houseCost;
            if (square -> hasHotel == 1) {
                replacementCost = square -> hotelCost;
            }
            int cost = (replacementCost * 25) / 100;

            if (cost <= player -> cash) {
                return i;
            }
        }

        i = i + 1;
    }

    return -1;
}

//Rule-LK 17: decides whether to renovate the property just landed on
int decideRenovateDepreciation(GameState *gamestate, int playerIndex, int squareIndex) {
    Player *player = &gamestate -> players[playerIndex];
    Square *square = &gamestate -> board[squareIndex];

    if (square -> depreciationPercentage == 0) {
        return 0;
    }

    int threshold = 0;
    if (player -> strategy == STRATEGY_CONSERVATIVE) {
        // Rule: Renovates depreciated properties immediately once depreciation exceeds 10%
        threshold = 10;
    } else if (player -> strategy == STRATEGY_OPPORTUNISTIC) {
        // Rule: Renovates properties once depreciation exceeds 15%.
        threshold = 15;
    } else if (player -> strategy == STRATEGY_RISK_TAKER) {
        /*Assumed - Ignores depreciation until repair becomes
        unavoidable is read as waiting until the 30% cap is reached. */
        threshold = DEPRECIATION_MAX_PERCENTAGE - 1;
    } else {
        /* Assumed- Aggressive Investor renovates as soon as any 
        depreciation exists, focusing on rents. */
        threshold = 0;
    }

    int wantsToRenovate = 0;
    if (square -> depreciationPercentage > threshold) {
        wantsToRenovate = 1;
    }

    if (wantsToRenovate == 1) {
        int cost = (square -> currentValue * 10) / 100;
        if (cost > player -> cash) {
            wantsToRenovate = 0;
        }
    }

    return wantsToRenovate;
}

/* Rule-LK 5: Most players extend a loan only when it's nearly due and
   can't be paid off. Risk Taker extends early instead, to keep more cash*/
int decideLoanExtension(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    if (player -> strategy == STRATEGY_RISK_TAKER) {
        if (player -> loanRoundsLeft <= 15) {
            return 1;
        }
        return 0;
    }

    int isRunningLow = 0;
    if (player -> loanRoundsLeft <= 5) {
        isRunningLow = 1;
    }

    int cantFullyRepay = 0;
    if (player -> cash < player -> loanAmount) {
        cantFullyRepay = 1;
    }

    int wantsToExtend = 0;
    if (isRunningLow == 1 && cantFullyRepay == 1) {
        wantsToExtend = 1;
    }
    return wantsToExtend;
}

/* Rule-LK 5: only Risk Taker proactively increases an existing loan,
matching "Frequently refinances loans to increase available capital." */
int decideLoanIncrease(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    if (player -> strategy != STRATEGY_RISK_TAKER) {
        return 0;
    }

    /* calculateMaxLoan() only counts unpledged, eligible collateral, so
    this is exactly how much more can be borrowed on top of the loan. */
    return calculateMaxLoan(gamestate, playerIndex);
}

// Rule 13: decides whether a jailed player pays bail voluntarily this turn.
int decideJailOrOut(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate -> players[playerIndex];

    int payBail = 0;

    if (player -> strategy == STRATEGY_AGGRESSIVE || player -> strategy == STRATEGY_RISK_TAKER) {
        // Wants back in the game fast, pays bail immediately if got sufficient cash
        if (player -> cash >= BAIL_AMOUNT) {
            payBail = 1;
        }

    } else if (player -> strategy == STRATEGY_CONSERVATIVE) {
        /* Assumption - Only pays if he would still be above the average cash 
           across all players after paying, else falls back to rolling doubles.*/
        int totalCash = 0;
        int i = 0;
        while (i < NUM_PLAYERS) {
            totalCash = totalCash + gamestate -> players[i].cash;
            i = i + 1;
        }
        int averageCash = totalCash / NUM_PLAYERS;

        if (player -> cash - BAIL_AMOUNT >= averageCash) {
            payBail = 1;
        }

    } else if (player -> strategy == STRATEGY_OPPORTUNISTIC) {
        /* Assumption - Only pays if a profitable market condition(Rule-LK 30/31 Market Boom) 
           is currently active, matching "evaluates expected return."
           Else falls back to rolling doubles.*/
        if (gamestate -> economy.marketBoom.isActive == 1 && player -> cash >= BAIL_AMOUNT) {
            payBail = 1;
        }
    }

    return payBail;
}