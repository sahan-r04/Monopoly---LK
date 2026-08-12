#include <stdio.h>
#include <stdlib.h>
#include "types.h"

// Implemented in finance.c (Rule-LK 2: 75% of the total mortgage value of eligible
// collateral) - declared here so players.c can call it too.
int calculateMaxLoan(GameState *gamestate, int playerIndex);


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

    switch (player -> strategy) {
        case STRATEGY_AGGRESSIVE: //always pays if got funds, at least one future rent
            return (player -> cash - square -> purchasePrice) >= square -> baseRent;

        case STRATEGY_CONSERVATIVE: //pays only if 50% of the current cash remains after paying 
            return (player -> cash - square -> purchasePrice) >= (player -> cash / 2);

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
        maxWillingToBid = (square -> currentValue * 120) / 100;

    } else if (player -> strategy == STRATEGY_CONSERVATIVE) {
        // Only bids below the market value
        maxWillingToBid = square -> currentValue;

    } else if (player -> strategy == STRATEGY_RISK_TAKER) {
        //Bids until cash is gone 
        maxWillingToBid = player -> cash;

    } else {
        //prefers discounted auction purchases only.
        maxWillingToBid = ((square -> currentValue * 80) / 100);
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
        // rental income." Only true if this player owns a monopoly group that
        // still has room to build - a new house/hotel there would raise that
        // group's rent (Table 6).
        int canImproveRentalIncome = 0;
        int i = 0;
        while (i < BOARD_SIZE) {
            Square *square = &gamestate -> board[i];

            int ownsThisSquare = 0;
            if (square -> ownerId == playerIndex) {
                ownsThisSquare = 1;
            }

            int hasRoomToBuild = 0;
            if (square -> numHouses < MAX_HOUSES && square -> hasHotel == 0) {
                hasRoomToBuild = 1;
            }

            int ownsMonopolyOnThisGroup = 0;
            if (square -> type == SQUARE_PROPERTY) {
                if (monopolyPlay(gamestate, playerIndex, square -> group) == 1) {
                    ownsMonopolyOnThisGroup = 1;
                }
            }

            if (ownsThisSquare == 1 && hasRoomToBuild == 1 && ownsMonopolyOnThisGroup == 1) {
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
        // Rule: "Avoids obtaining loans unless bankruptcy is imminent."
        // ASSUMPTION - the brief never defines "imminent," so this treats cash
        // falling below 5% of the starting cash as the danger line.
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
        // STRATEGY_OPPORTUNISTIC - Rule: "Obtains loans only when projected
        // return exceeds borrowing cost."
        // ASSUMPTION - "projected return" is approximated as this player's
        // existing total rental income per round (their current earning power);
        // "borrowing cost" is one round of interest on the max loan, the same
        // Current Loan x Current Interest Rate formula applyLoanInterest() in
        // finance.c already uses (Rule-LK 4).
        int projectedReturn = 0;
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].ownerId == playerIndex) {
                projectedReturn = projectedReturn + gamestate -> board[i].currentRent;
            }
            i = i + 1;
        }

        int borrowingCost = (maxLoan * gamestate -> economy.baseInterestRate) / 100;

        if (projectedReturn > borrowingCost) {
            return maxLoan / 3;
        } else {
            return 0;
        }
    }
}