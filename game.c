#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

// Rule 1: sets up 4 players with starting values. Call once at game start.
void initializingPlayers(GameState *gamestate) {
    const char *names[4] = {"Aggressive Investor", "Conservative Banker", "Risk Taker", "Opportunistic Trader" };
    StrategyType strategies[4] = { STRATEGY_AGGRESSIVE, STRATEGY_CONSERVATIVE, STRATEGY_RISK_TAKER, STRATEGY_OPPORTUNISTIC };

    for (int i = 0; i < NUM_PLAYERS; i++) {
        Player *player = &gamestate->players[i];
        strncpy(player -> name, names[i], MAX_NAME_LEN);
        player -> strategy = strategies[i];
        player -> cash = STARTING_CASH;
        player -> position = 0;
        player -> inJail = 0;
        player -> isBankrupt = 0;
        player -> hasLoan = 0;
    }
}

// Rule 2: each player rolls 2 dice, highest goes first. Fills 'order' with player indices.
void determineTurnOrder(GameState *gamestate, int order[NUM_PLAYERS]) {
    int rolls[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        int die1 = rand() % 6 + 1;
        int die2 = rand() % 6 + 1;
        rolls[i] = die1 + die2;
        printf("%s rolls %d.\n", gamestate->players[i].name, rolls[i]);
    }

    // Simple selection sort by roll value, descending, into 'order'
    for (int i = 0; i < NUM_PLAYERS; i++) {
        order[i] = i;
    }
    for (int i = 0; i < NUM_PLAYERS - 1; i++) {
        for (int j = i + 1; j < NUM_PLAYERS; j++) {
            if (rolls[order[j]] > rolls[order[i]]) {
                int temporaryHolder = order[i];
                order[i] = order[j];
                order[j] = temporaryHolder;
            }
        }
    }

    // Rule 2: if two or more players tied for a position, only THOSE players
    // re-roll until broken - everyone else stays exactly where the sort put them.
    int i = 0;
    while (i < NUM_PLAYERS) {

        // Find how many players in a row, starting at position i, are tied
        // with whoever is at position i.
        int tieGroupSize = 1;
        int j = i + 1;
        while (j < NUM_PLAYERS && rolls[order[j]] == rolls[order[i]]) {
            tieGroupSize = tieGroupSize + 1;
            j = j + 1;
        }

        if (tieGroupSize > 1) {

            // Keep re-rolling just this tied group and re-sorting it until no
            // two of them end up next to each other with the same roll.
            int tieBroken = 0;
            while (tieBroken == 0) {

                // Re-roll every player in this tied group.
                int k = i;
                while (k < i + tieGroupSize) {
                    int tieDie1 = rand() % 6 + 1;
                    int tieDie2 = rand() % 6 + 1;
                    rolls[order[k]] = tieDie1 + tieDie2;
                    printf("%s rolls again : %d.\n", gamestate -> players[order[k]].name, rolls[order[k]]);
                    k = k + 1;
                }

                // Sort just this tied group (positions i .. i+tieGroupSize-1)
                // by their new rolls, the same selection-sort idea used above.
                int a = i;
                while (a < i + tieGroupSize - 1) {
                    int b = a + 1;
                    while (b < i + tieGroupSize) {
                        if (rolls[order[b]] > rolls[order[a]]) {
                            int temporaryHolder = order[a];
                            order[a] = order[b];
                            order[b] = temporaryHolder;
                        }
                        b = b + 1;
                    }
                    a = a + 1;
                }

                // After sorting, any players still tied are now guaranteed to
                // be next to each other - so it's enough to check neighbours.
                tieBroken = 1;
                int c = i;
                while (c < i + tieGroupSize - 1) {
                    if (rolls[order[c]] == rolls[order[c + 1]]) {
                        tieBroken = 0;
                    }
                    c = c + 1;
                }
            }
        }

        i = i + tieGroupSize;
    }
}

// Rule 3: the full sequence for ONE player's turn.
void playTurn(GameState *gamestate, int playerIndex) {
    Player *player = &gamestate->players[playerIndex];
    if (player -> isBankrupt == 1) {
        return;
    }

    // 1. Resolve outstanding penalties (jail check) - Rule 13.
    if (player -> inJail == 1) {

        int jailDie1 = rand() % 6 + 1;
        int jailDie2 = rand() % 6 + 1;

        int rolledDoubles = 0;
        if (jailDie1 == jailDie2) {
            rolledDoubles = 1;
        }

        if (rolledDoubles == 1) {
            // Rolling doubles gets the player out immediately, free of charge.
            player -> inJail = 0;
            player -> jailTurns = 0;
            printf("%s rolled doubles (%d, %d) and leaves Jail.\n", player -> name, jailDie1, jailDie2);

        } else {
            player -> jailTurns = player -> jailTurns + 1;

            int mustPayBailNow = 0;
            if (player -> jailTurns >= MAX_JAIL_TURNS) {
                mustPayBailNow = 1;
            }

            if (mustPayBailNow == 1) {
                // After three turns imprisoned, the player must pay bail and leave.
                player -> cash = player -> cash - BAIL_AMOUNT;
                player -> inJail = 0;
                player -> jailTurns = 0;
                printf("%s paid bail of LKR %d and leaves Jail.\n", player -> name, BAIL_AMOUNT);
            } else {
                printf("%s stays in Jail.\n", player -> name);
                return; // skip rest of turn if still jailed
            }
        }
    }

    // 2. Roll two dice
    int die1 = rand() % 6 + 1;
    int die2 = rand() % 6 + 1;
    int diceTotal = die1 + die2;
    printf("%s rolled %d.\n", player -> name, diceTotal);

    // 3. Move clockwise
    int passedGo = movePlayer(player, diceTotal); // from board.c
    if (passedGo == 1) {
        printf("%s passed GO.\nCollected LKR %d.\nCurrent Balance : LKR %d.\n", player -> name, PASSING_GO_PAY, player -> cash);
    }

    // 4. Resolve landing action (dispatch based on square type)
    Square *square = &gamestate->board[player -> position];
    switch (square -> type) {
        case SQUARE_PROPERTY:
        case SQUARE_RAILWAY:
        case SQUARE_UTILITY:
            // 5. Purchase property if eligible
            if (square -> ownerId == -1) {
                if (decideToPurchase(gamestate, playerIndex, player -> position) == 1) { // players.c
                    player -> cash = (player -> cash) - (square -> purchasePrice);
                    square -> ownerId = playerIndex;
                    printf("%s purchased %s for LKR %d.\n", player -> name, square -> name, square -> purchasePrice);
                } else {
                    runAuction(gamestate, player -> position); // declined -> auction (Rule 5)
                }
            } else if (square -> ownerId != playerIndex) {
                // Table 2/Section 1.1.3: railway/utility rent depends on ownership
                // count/dice at the moment of payment, so recalculate it fresh here.
                if (square -> type == SQUARE_RAILWAY) {
                    square -> currentRent = calculateRailwayRent(gamestate, player -> position); // finance.c
                } else if (square -> type == SQUARE_UTILITY) {
                    square -> currentRent = calculateUtilityRent(gamestate, player -> position, diceTotal); // finance.c
                }
                payRent(gamestate, playerIndex, player -> position); // finance.c
            }
            break;

        case SQUARE_TAX:
            collectTax(gamestate, playerIndex); // finance.c - Rule 11
            break;

        case SQUARE_GO_TO_JAIL:
            sendToJail(player); // board.c
            break;

        case SQUARE_EVENT:
            drawNationalEventCard(gamestate, playerIndex); // events.c
            break;

        case SQUARE_BANK:
            // Rule-LK 5: repay an existing loan, or take out a new one.
            if (player -> hasLoan == 1) {
                int repayAmount = decideLoanRepaymentAmount(gamestate, playerIndex); // players.c
                if (repayAmount > 0) {
                    repayLoan(gamestate, playerIndex, repayAmount); // finance.c
                    printf("%s repaid LKR %d.\n", player -> name, repayAmount);
                }
            } else {
                int loanAmount = decideLoanAmount(gamestate, playerIndex); // players.c
                if (loanAmount > 0) {

                    // ASSUMPTION - pledges every eligible (unmortgaged, unlocked)
                    // owned square as collateral, the same set calculateMaxLoan() uses.
                    int collateralChoice[BOARD_SIZE];
                    int c = 0;
                    while (c < BOARD_SIZE) {
                        Square *collateralSquare = &gamestate -> board[c];
                        if (collateralSquare -> ownerId == playerIndex && collateralSquare -> isMortgaged == 0 && collateralSquare -> isLoanLocked == 0) {
                            collateralChoice[c] = 1;
                        } else {
                            collateralChoice[c] = 0;
                        }
                        c = c + 1;
                    }

                    takeLoan(gamestate, playerIndex, loanAmount, collateralChoice); // finance.c
                    printf("%s obtained a secured loan of LKR %d.\n", player -> name, loanAmount);
                }
            }
            break;

        case SQUARE_INSURANCE:
            // Section 2.2: pick a property to insure, and which policy to buy.
            {
                int targetSquareIndex = decideInsuranceTarget(gamestate, playerIndex); // players.c
                if (targetSquareIndex != -1) {
                    InsuranceType chosenPolicy = decideInsurancePolicy(gamestate, playerIndex, targetSquareIndex); // players.c
                    if (chosenPolicy != INSURANCE_NONE) {
                        getInsurance(gamestate, playerIndex, targetSquareIndex, chosenPolicy); // finance.c
                        printf("%s purchased insurance for %s.\n", player -> name, gamestate -> board[targetSquareIndex].name);
                    }
                }
            }
            break;

        default:
            break; // SQUARE_START, SQUARE_FREE_PARKING, SQUARE_JAIL (just visiting) — nothing to do
    }

    // 6. Construct buildings if eligible (Rule 8/9).
    int buildSquareIndex = decideConstruction(gamestate, playerIndex); // players.c
    if (buildSquareIndex != -1) {
        Square *buildSquare = &gamestate -> board[buildSquareIndex];

        int isUpgradingToHotel = 0;
        if (buildSquare -> numHouses == MAX_HOUSES) {
            isUpgradingToHotel = 1;
        }

        int oldRentMultiplier = getRentMultiplier(buildSquare); // board.c

        if (isUpgradingToHotel == 1) {
            player -> cash = player -> cash - buildSquare -> hotelCost;
            buildSquare -> numHouses = 0;
            buildSquare -> hasHotel = 1;
            printf("%s upgraded %s to a Hotel.\n", player -> name, buildSquare -> name);
        } else {
            player -> cash = player -> cash - buildSquare -> houseCost;
            buildSquare -> numHouses = buildSquare -> numHouses + 1;
            printf("%s constructed one house on %s.\nConstruction Cost : LKR %d.\n", player -> name, buildSquare -> name, buildSquare -> houseCost);
        }

        int newRentMultiplier = getRentMultiplier(buildSquare); // board.c
        recalculateRentAfterConstruction(buildSquare, oldRentMultiplier, newRentMultiplier); // board.c
    }

    // 7. Complete financial transactions - mortgage a property if needed.
    int mortgageSquareIndex = decideMortgage(gamestate, playerIndex); // players.c
    if (mortgageSquareIndex != -1) {
        Square *mortgageSquare = &gamestate -> board[mortgageSquareIndex];
        mortgageSquare -> isMortgaged = 1;
        player -> cash = player -> cash + mortgageSquare -> mortgageValue;
        printf("%s mortgaged %s for LKR %d.\n", player -> name, mortgageSquare -> name, mortgageSquare -> mortgageValue);
    }

    // 8. End turn — nothing more to do, function returns
}

// Rule 6, Rule-LK 19-23: runs an auction for one property among all solvent players.
void runAuction(GameState *gamestate, int squareIndex) {

    Square *square = &gamestate -> board[squareIndex];

    // 1) Opening bid is 50% of market value (Rule-LK 19).
    int currentBid = square -> currentValue / 2;
    int highestBidder = -1;  // -1 means nobody has bid yet

    printf("Auction Started.\n\nProperty :\n%s\n\nOpening Bid :\nLKR %d.\n\n", square -> name, currentBid);

    // 2) Bankrupt players never take part (Rule 6). Everyone else starts in.
    int stillInGame[NUM_PLAYERS];
    int p = 0;
    while (p < NUM_PLAYERS) {
        if (gamestate -> players[p].isBankrupt == 1) {
            stillInGame[p] = 0;
        } else {
            stillInGame[p] = 1;
        }
        p = p + 1;
    }

    // 3) Keep going around the table until only the current leader (or nobody) is
    //    left standing - a withdrawal is permanent (Rule-LK 21).
    int auctionOngoing = 1;
    while (auctionOngoing == 1) {

        int somebodyRaisedThisPass = 0;
        int i = 0;
        while (i < NUM_PLAYERS) {

            // The current leader doesn't need to bid against themselves - without this
            // check they'd fail to beat their own bid next pass and be wrongly eliminated.
            if (stillInGame[i] == 1 && i != highestBidder) {

                int bid = decideAuctionBid(gamestate, i, squareIndex, currentBid);

                // A real bid must clear the minimum increment (Rule-LK 20) and never
                // exceed the player's cash on hand (Rule-LK 22).
                int isValidBid = 0;
                if (bid >= currentBid + AUCTION_INCREMENT && bid <= gamestate -> players[i].cash) {
                    isValidBid = 1;
                }

                if (isValidBid == 1) {
                    currentBid = bid;
                    highestBidder = i;
                    somebodyRaisedThisPass = 1;
                    printf("%s bids LKR %d.\n", gamestate -> players[i].name, currentBid);
                } else {
                    stillInGame[i] = 0;
                    printf("%s withdraws.\n", gamestate -> players[i].name);
                }
            }

            i = i + 1;
        }

        // 4) Count how many players are still standing.
        int remaining = 0;
        int j = 0;
        while (j < NUM_PLAYERS) {
            if (stillInGame[j] == 1) {
                remaining = remaining + 1;
            }
            j = j + 1;
        }

        if (remaining <= 1) {
            auctionOngoing = 0;
        }
        if (somebodyRaisedThisPass == 0) {
            auctionOngoing = 0;
        }
    }

    // 5) Award the property, or leave it with the Bank if nobody ever bid (Rule-LK 23).
    if (highestBidder == -1) {
        printf("\nNo bids were placed. Ownership remains with the Bank.\n");
    } else {
        gamestate -> players[highestBidder].cash = (gamestate -> players[highestBidder].cash) - (currentBid);
        square -> ownerId = highestBidder;
        printf("\n%s wins the auction.\n", gamestate -> players[highestBidder].name);
    }
}

// Called once at the very end of every round — interest, depreciation, expiries, market ticks.
void endOfRoundProcessing(GameState *gamestate) {
    gamestate -> economy.round = gamestate -> economy.round + 1;
    applyLoanInterest(gamestate);        // finance.c

    if (gamestate -> economy.round % 10 == 0) {
        inflationChange(gamestate);          // events.c
        propertyMarketChange(gamestate);     // events.c
        triggerDisaster(gamestate);          // events.c
    }
    if (gamestate -> economy.round % 15 == 0) {
        drawRegionalDevelopmentCard(gamestate);  // events.c
        drawEconomicEvent(gamestate);            // events.c - Rule-LK 18
    }
    if (gamestate -> economy.round % 20 == 0) {
        governmentRegulationChange(gamestate);   // events.c
    }

    applyRoundTickEffects(gamestate);    // finance.c — Rule-LK 9/15/16/25/26/33
    revertExpiredEffects(gamestate);     // events.c — Rule-LK 35
    reopenClosedSquares(gamestate);      // board.c — Political Rally reopening
    checkBankruptcy(gamestate);          // finance.c
    printRoundSummary(gamestate);
    displayMarketConditions(gamestate);  // events.c - Rule-LK 36
}

// Section 5 "End of Every Round" output - prints every player's status.
void printRoundSummary(GameState *gamestate) {

    printf("=============================================\n");
    printf("Round %d Summary\n", gamestate -> economy.round);
    printf("=============================================\n");

    int p = 0;
    while (p < NUM_PLAYERS) {
        Player *player = &gamestate -> players[p];

        // Count this player's properties and hotels by scanning the board.
        int propertyCount = 0;
        int hotelCount = 0;
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].ownerId == p) {
                propertyCount = propertyCount + 1;
                if (gamestate -> board[i].hasHotel == 1) {
                    hotelCount = hotelCount + 1;
                }
            }
            i = i + 1;
        }

        printf("%s\n", player -> name);
        printf("Cash : LKR %d\n", player -> cash);
        printf("Net Worth : LKR %d\n", calculateNetWorth(gamestate, p));
        printf("Properties : %d\n", propertyCount);
        printf("Hotels : %d\n", hotelCount);

        if (player -> hasLoan == 1) {
            printf("Outstanding Loan : LKR %d\n", player -> loanAmount);
        } else {
            printf("Outstanding Loan : None\n");
        }

        if (p < NUM_PLAYERS - 1) {
            printf("---------------------------------------------\n");
        }

        p = p + 1;
    }

    printf("=============================================\n");
}

// Counts remaining solvent players; returns 1 if the game should stop.
int isGameOver(GameState *gamestate) {
    int remainingPlayers = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (gamestate -> players[i].isBankrupt == 0) {
            remainingPlayers = remainingPlayers + 1;
        }
    }

    int gameShouldStop = 0;
    if (remainingPlayers <= 1) {
        gameShouldStop = 1;
    }
    if (gamestate -> economy.round >= MAX_ROUNDS) {
        gameShouldStop = 1;
    }
    return gameShouldStop;
}

// Rule 15: picks the player with the highest net worth.
int findWinner(GameState *gamestate) {
    int winner = 0;
    int highestNetWorth = calculateNetWorth(gamestate, 0); // finance.c

    for (int i = 1; i < NUM_PLAYERS; i++) {
        if (gamestate -> players[i].isBankrupt == 0) {
            int netWorth = calculateNetWorth(gamestate, i);
            if (netWorth > highestNetWorth) {
                highestNetWorth = netWorth;
                winner = i;
            }
        }
    }
    return winner;
}

// Runs the whole simulation end-to-end - board/player setup, turn order, the
// main game loop (Rule 15), and announcing the winner. Called once from main().
void runGame(GameState *gamestate) {
    printf("MONOPOLY-LK Simulation\n\n");

    monopolyBoard(gamestate -> board);
    initializingPlayers(gamestate);

    // ASSUMPTION - starting loan interest rate uses Table 9's "Stable
    // Economy" rate (8%), since the game starts under normal conditions and
    // no rule states a starting number directly; currentIncomeTaxAmount
    // seeds from the same constant Rule 11 collectTax() reads every round.
    gamestate -> economy.baseInterestRate = 8;
    gamestate -> economy.currentIncomeTaxAmount = INCOME_TAX_AMOUNT;

    int turnOrder[NUM_PLAYERS];
    determineTurnOrder(gamestate, turnOrder);

    printf("\nTurn order:\n");
    int i = 0;
    while (i < NUM_PLAYERS) {
        printf("%s\n", gamestate -> players[turnOrder[i]].name);
        i = i + 1;
    }

    // Main game loop - Rule 15 (max 500 rounds or 1 solvent player left).
    // A round is NOT simply "every player took one turn" - players move
    // different distances each turn, so a round only completes once the
    // player who is furthest BEHIND on the board (fewest laps of GO, not
    // turn order) has passed GO. Round-based effects (interest, inflation,
    // disasters, card draws, depreciation, etc.) all key off this.
    while (isGameOver(gamestate) == 0) {
        int p = 0;
        while (p < NUM_PLAYERS) {
            playTurn(gamestate, turnOrder[p]);

            int slowestLaps = -1;
            int k = 0;
            while (k < NUM_PLAYERS) {
                if (gamestate -> players[k].isBankrupt == 0) {
                    if (slowestLaps == -1 || gamestate -> players[k].lapsCompleted < slowestLaps) {
                        slowestLaps = gamestate -> players[k].lapsCompleted;
                    }
                }
                k = k + 1;
            }

            if (slowestLaps > gamestate -> economy.round) {
                endOfRoundProcessing(gamestate);
            }

            p = p + 1;
        }
    }

    int winner = findWinner(gamestate);
    printf("\n=============================================\n");
    printf("GAME OVER\n\nWinner\n\n%s\n\nNet Worth\n\nLKR %d\n", gamestate -> players[winner].name, calculateNetWorth(gamestate, winner));
    printf("=============================================\n");
}
