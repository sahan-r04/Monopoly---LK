#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "types.h"

//---- Rule-LK 2 ----//
int calculateMaxLoan(GameState *gs, int playerIndex){

    int totalMortgageValue = 0;

    int i = 0;

    while (i < BOARD_SIZE){
        Square landedSquare = gs -> board[i];

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
