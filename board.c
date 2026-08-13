#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "types.h"



void monopolyBoard(Square board[BOARD_SIZE]) {

    // ---- 0. GO ----//
    board[0] = (Square){
        .squareIndex = 0,
        .name = "GO",
        .type = SQUARE_START,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    //---- 1. Pettah (Property-Brown) ----//
    board[1] = (Square){
        .squareIndex = 1,
        .name = "Pettah",
        .type = SQUARE_PROPERTY,
        .group = GROUP_BROWN,

        .purchasePrice = 1500,
        .baseRent = 100,
        .houseCost = 500,
        .hotelCost = 2000,
        .mortgageValue = 750,

        .currentValue = 1500,
        .currentRent = 100,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    //---- 2. Community Development Fund (Event) ----//
    board[2] = (Square){
        .squareIndex = 2,
        .name = "Community Development Fund",
        .type = SQUARE_EVENT,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    //---- 3. Maradana (Property-Brown) ---- */
    board[3] = (Square){
        .squareIndex = 3,
        .name = "Maradana",
        .type = SQUARE_PROPERTY,
        .group = GROUP_BROWN,

        .purchasePrice = 1500,   /* was 1800 — Brown group price is fixed at 1500 (Appendix B) */
        .baseRent = 100,
        .houseCost = 500,
        .hotelCost = 2000,
        .mortgageValue = 750,

        .currentValue = 1500,
        .currentRent = 100,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 4. Income Tax ---- //
    board[4] = (Square){
        .squareIndex = 4,
        .name = "Income Tax",
        .type = SQUARE_TAX,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 5. Colombo Fort Railway Station ---- //
    board[5] = (Square){
        .squareIndex = 5,
        .name = "Colombo Fort Railway Station",
        .type = SQUARE_RAILWAY,
        .group = GROUP_NONE,

        .purchasePrice = 2500,   /* ASSUMPTION — not given in the brief */
        .mortgageValue = 1250,   /* ASSUMPTION — not given in the brief */
        .baseRent = 250,          /* Table 2: rent for 1 station owned — this IS given */
        /* no houseCost/hotelCost — railways cannot be developed (1.1.2) */

        .currentValue = 2500,
        .currentRent = 250,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,
        /* numHouses/hasHotel/buildingCondition/propertyAge/isDamaged/insurance
           all left at zero-default — none of it applies to a railway */
    };

    // ---- 6. Bambalapitiya (Property-Light Blue) ---- //
    board[6] = (Square){
        .squareIndex = 6,
        .name = "Bambalapitiya",
        .type = SQUARE_PROPERTY,
        .group = GROUP_LIGHT_BLUE,

        .purchasePrice = 2500,
        .baseRent = 150,
        .houseCost = 750,
        .hotelCost = 3000,
        .mortgageValue = 1250,

        .currentValue = 2500,
        .currentRent = 150,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 7. National Event Card (Event) ---- //
    board[7] = (Square){
        .squareIndex = 7,
        .name = "National Event Card",
        .type = SQUARE_EVENT,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 8. Wellawatte (Property-Light Blue) ---- //
    board[8] = (Square){
        .squareIndex = 8,
        .name = "Wellawatte",
        .type = SQUARE_PROPERTY,
        .group = GROUP_LIGHT_BLUE,

        .purchasePrice = 2500,
        .baseRent = 150,
        .houseCost = 750,
        .hotelCost = 3000,
        .mortgageValue = 1250,

        .currentValue = 2500,
        .currentRent = 150,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 9: Mount Lavinia (Property-Light Blue) ---- //
    board[9] = (Square){
        .squareIndex = 9,
        .name = "Mount Lavinia",
        .type = SQUARE_PROPERTY,
        .group = GROUP_LIGHT_BLUE,

        .purchasePrice = 2500,
        .baseRent = 150,
        .houseCost = 750,
        .hotelCost = 3000,
        .mortgageValue = 1250,

        .currentValue = 2500,
        .currentRent = 150,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 10. Jail (Just Visiting) ---- //
    board[10] = (Square){
        .squareIndex = 10,
        .name = "Jail / Just Visiting",
        .type = SQUARE_JAIL,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 11. Nugegoda (Property-Pink) ---- //
    board[11] = (Square){
        .squareIndex = 11,
        .name = "Nugegoda",
        .type = SQUARE_PROPERTY,
        .group = GROUP_PINK,

        .purchasePrice = 3500,
        .baseRent = 220,
        .houseCost = 1000,
        .hotelCost = 4000,
        .mortgageValue = 1750,

        .currentValue = 3500,
        .currentRent = 220,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 12. Ceylon Electricity Board (Property-Utility) ---- //
    board[12] = (Square){
        .squareIndex = 12,
        .name = "Ceylon Electricity Board",
        .type = SQUARE_UTILITY,
        .group = GROUP_NONE,

        .purchasePrice = 1500,   /* ASSUMPTION — not given in the brief */
        .mortgageValue = 750,    /* ASSUMPTION — not given in the brief */
        .baseRent = 0,           /* rent is dice-based (Section 1.1.3), computed live, not stored here */

        .currentValue = 1500,
        .currentRent = 0,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,
    };

    // ---- 13. Maharagama (Property-Pink) ---- //
    board[13] = (Square){
        .squareIndex = 13,
        .name = "Maharagama",
        .type = SQUARE_PROPERTY,
        .group = GROUP_PINK,

        .purchasePrice = 3500,
        .baseRent = 220,
        .houseCost = 1000,
        .hotelCost = 4000,
        .mortgageValue = 1750,

        .currentValue = 3500,
        .currentRent = 220,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 14. Kottawa (Property-Pink) ---- //
    board[14] = (Square){
        .squareIndex = 14,
        .name = "Kottawa",
        .type = SQUARE_PROPERTY,
        .group = GROUP_PINK,

        .purchasePrice = 3500,
        .baseRent = 220,
        .houseCost = 1000,
        .hotelCost = 4000,
        .mortgageValue = 1750,

        .currentValue = 3500,
        .currentRent = 220,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 15. Kandy Railway Station ---- //
    board[15] = (Square){
        .squareIndex = 15,
        .name = "Kandy Railway Station",
        .type = SQUARE_RAILWAY,
        .group = GROUP_NONE,

        .purchasePrice = 2500,
        .mortgageValue = 1250,
        .baseRent = 250,

        .currentValue = 2500,
        .currentRent = 250,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,
    };

    // ---- 16. Negombo (Property-Orange) ---- //
    board[16] = (Square){
        .squareIndex = 16,
        .name = "Negombo",
        .type = SQUARE_PROPERTY,
        .group = GROUP_ORANGE,

        .purchasePrice = 4500,
        .baseRent = 300,
        .houseCost = 1250,
        .hotelCost = 5000,
        .mortgageValue = 2250,

        .currentValue = 4500,
        .currentRent = 300,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 17. Sri Lanka Insurance ---- //
    board[17] = (Square){
        .squareIndex = 17,
        .name = "Sri Lanka Insurance",
        .type = SQUARE_INSURANCE,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 18. Katunayake (Property-Orange) ---- //
    board[18] = (Square){
        .squareIndex = 18,
        .name = "Katunayake",
        .type = SQUARE_PROPERTY,
        .group = GROUP_ORANGE,

        .purchasePrice = 4500,
        .baseRent = 300,
        .houseCost = 1250,
        .hotelCost = 5000,
        .mortgageValue = 2250,

        .currentValue = 4500,
        .currentRent = 300,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 19. Ja-Ela (Property-Orange) ---- //
    board[19] = (Square){
        .squareIndex = 19,
        .name = "Ja-Ela",
        .type = SQUARE_PROPERTY,
        .group = GROUP_ORANGE,

        .purchasePrice = 4500,
        .baseRent = 300,
        .houseCost = 1250,
        .hotelCost = 5000,
        .mortgageValue = 2250,

        .currentValue = 4500,
        .currentRent = 300,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 20. Free Parking (Special) ---- //
    board[20] = (Square){
        .squareIndex = 20,
        .name = "Free Parking",
        .type = SQUARE_FREE_PARKING,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 21. Kandy City (Property-Red) ----//
    board[21] = (Square){
        .squareIndex = 21,
        .name = "Kandy City",
        .type = SQUARE_PROPERTY,
        .group = GROUP_RED,

        .purchasePrice = 5500,
        .baseRent = 380,
        .houseCost = 1500,
        .hotelCost = 6000,
        .mortgageValue = 2750,

        .currentValue = 5500,
        .currentRent = 380,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 22. National Event Card (Event) ---- //
    board[22] = (Square){
        .squareIndex = 22,
        .name = "National Event Card",
        .type = SQUARE_EVENT,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 23. Peradeniya (Property-Red) ---- //
    board[23] = (Square){
        .squareIndex = 23,
        .name = "Peradeniya",
        .type = SQUARE_PROPERTY,
        .group = GROUP_RED,

        .purchasePrice = 5500,
        .baseRent = 380,
        .houseCost = 1500,
        .hotelCost = 6000,
        .mortgageValue = 2750,

        .currentValue = 5500,
        .currentRent = 380,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 24. Katugastota (Property-Red) ---- //
    board[24] = (Square){
        .squareIndex = 24,
        .name = "Katugastota",
        .type = SQUARE_PROPERTY,
        .group = GROUP_RED,

        .purchasePrice = 5500,
        .baseRent = 380,
        .houseCost = 1500,
        .hotelCost = 6000,
        .mortgageValue = 2750,

        .currentValue = 5500,
        .currentRent = 380,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 25. Galle Railway Station ---- //
    board[25] = (Square){
        .squareIndex = 25,
        .name = "Galle Railway Station",
        .type = SQUARE_RAILWAY,
        .group = GROUP_NONE,

        .purchasePrice = 2500,
        .mortgageValue = 1250,
        .baseRent = 250,

        .currentValue = 2500,
        .currentRent = 250,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,
    };

    // ---- 26. Galle Fort (Property-Yellow) ---- //
    board[26] = (Square){
        .squareIndex = 26,
        .name = "Galle Fort",
        .type = SQUARE_PROPERTY,
        .group = GROUP_YELLOW,

        .purchasePrice = 6500,
        .baseRent = 460,
        .houseCost = 2000,
        .hotelCost = 8000,
        .mortgageValue = 3250,

        .currentValue = 6500,
        .currentRent = 460,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 27. Unawatuna (Property-Yellow) ---- //
    board[27] = (Square){
        .squareIndex = 27,
        .name = "Unawatuna",
        .type = SQUARE_PROPERTY,
        .group = GROUP_YELLOW,

        .purchasePrice = 6500,
        .baseRent = 460,
        .houseCost = 2000,
        .hotelCost = 8000,
        .mortgageValue = 3250,

        .currentValue = 6500,
        .currentRent = 460,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 28. National Water Supply and Drainage Board (Utility) ---- //
    board[28] = (Square){
        .squareIndex = 28,
        .name = "National Water Supply and Drainage Board",
        .type = SQUARE_UTILITY,
        .group = GROUP_NONE,

        .purchasePrice = 1500,
        .mortgageValue = 750,
        .baseRent = 0,

        .currentValue = 1500,
        .currentRent = 0,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,
    };

    // ---- 29. Hikkaduwa (Property-Yellow) ---- //
    board[29] = (Square){
        .squareIndex = 29,
        .name = "Hikkaduwa",
        .type = SQUARE_PROPERTY,
        .group = GROUP_YELLOW,

        .purchasePrice = 6500,
        .baseRent = 460,
        .houseCost = 2000,
        .hotelCost = 8000,
        .mortgageValue = 3250,

        .currentValue = 6500,
        .currentRent = 460,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 30. Go To Jail (Special) ---- //
    board[30] = (Square){
        .squareIndex = 30,
        .name = "Go To Jail",
        .type = SQUARE_GO_TO_JAIL,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 31. Jaffna Town (Property-Green) ---- //
    board[31] = (Square){
        .squareIndex = 31,
        .name = "Jaffna Town",
        .type = SQUARE_PROPERTY,
        .group = GROUP_GREEN,

        .purchasePrice = 8000,
        .baseRent = 550,
        .houseCost = 2500,
        .hotelCost = 10000,
        .mortgageValue = 4000,

        .currentValue = 8000,
        .currentRent = 550,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 32. Nallur (Property-Green) ---- //
    board[32] = (Square){
        .squareIndex = 32,
        .name = "Nallur",
        .type = SQUARE_PROPERTY,
        .group = GROUP_GREEN,

        .purchasePrice = 8000,
        .baseRent = 550,
        .houseCost = 2500,
        .hotelCost = 10000,
        .mortgageValue = 4000,

        .currentValue = 8000,
        .currentRent = 550,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 33. Ceylinco Insurance ---- //
    board[33] = (Square){
        .squareIndex = 33,
        .name = "Ceylinco Insurance",
        .type = SQUARE_INSURANCE,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 34. Trincomalee (Property-Green) ---- //
    board[34] = (Square){
        .squareIndex = 34,
        .name = "Trincomalee",
        .type = SQUARE_PROPERTY,
        .group = GROUP_GREEN,

        .purchasePrice = 8000,
        .baseRent = 550,
        .houseCost = 2500,
        .hotelCost = 10000,
        .mortgageValue = 4000,

        .currentValue = 8000,
        .currentRent = 550,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 35. Jaffna Railway Station ---- //
    board[35] = (Square){
        .squareIndex = 35,
        .name = "Jaffna Railway Station",
        .type = SQUARE_RAILWAY,
        .group = GROUP_NONE,

        .purchasePrice = 2500,
        .mortgageValue = 1250,
        .baseRent = 250,

        .currentValue = 2500,
        .currentRent = 250,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,
    };

    // ---- 36. National Event Card (Event) ---- //
    board[36] = (Square){
        .squareIndex = 36,
        .name = "National Event Card",
        .type = SQUARE_EVENT,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 37. Nuwara Eliya (Property-Dark Blue) ---- //
    board[37] = (Square){
        .squareIndex = 37,
        .name = "Nuwara Eliya",
        .type = SQUARE_PROPERTY,
        .group = GROUP_DARK_BLUE,

        .purchasePrice = 10000,
        .baseRent = 650,
        .houseCost = 3000,
        .hotelCost = 12000,
        .mortgageValue = 5000,

        .currentValue = 10000,
        .currentRent = 650,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };

    // ---- 38. Bank of Ceylon ---- //
    board[38] = (Square){
        .squareIndex = 38,
        .name = "Bank of Ceylon",
        .type = SQUARE_BANK,
        .group = GROUP_NONE,
        .ownerId = -1,
    };

    // ---- 39. Galle Face (Property-Dark Blue) ---- //
    board[39] = (Square){
        .squareIndex = 39,
        .name = "Galle Face",
        .type = SQUARE_PROPERTY,
        .group = GROUP_DARK_BLUE,

        .purchasePrice = 10000,
        .baseRent = 650,
        .houseCost = 3000,
        .hotelCost = 12000,
        .mortgageValue = 5000,

        .currentValue = 10000,
        .currentRent = 650,

        .ownerId = -1,
        .isMortgaged = 0,
        .isLoanLocked = 0,

        .numHouses = 0,
        .hasHotel = 0,

        .buildingCondition = 100,
        .propertyAge = 0,
        .isDamaged = 0,

        .insurancePolicy = INSURANCE_NONE,
        .insuranceRoundsLeft = 0,
    };
}

int movePlayer(Player *p, int diceTotal) {
    int oldPosition = p->position;
    int newPosition = (oldPosition + diceTotal) % BOARD_SIZE;

    int passedGo = (newPosition < oldPosition);
    p->position = newPosition;

    if (passedGo) {
        p -> cash =  p -> cash + PASSING_GO_PAY; // Add cash for passing GO
    }
    return passedGo;
}

void  sendToJail(Player *p){
    p->position = 10;
    p->inJail = 1;
    p->jailTurns = 0;
}

// Table 6: the rent multiplier for a property's current development level.
int getRentMultiplier(Square *square) {
    int multiplier = 1;

    if (square -> hasHotel == 1) {
        multiplier = 10;
    } else if (square -> numHouses == 4) {
        multiplier = 7;
    } else if (square -> numHouses == 3) {
        multiplier = 5;
    } else if (square -> numHouses == 2) {
        multiplier = 3;
    } else if (square -> numHouses == 1) {
        multiplier = 2;
    } else {
        multiplier = 1;
    }

    return multiplier;
}

// Called right after a house/hotel is added, with the multiplier from just
// before and just after. Scales currentRent by that ratio instead of
// recalculating from baseRent, so any active card/regulation rent effects
// already baked into currentRent aren't wiped out by the new construction.
void recalculateRentAfterConstruction(Square *square, int oldMultiplier, int newMultiplier) {
    square -> currentRent = (square -> currentRent * newMultiplier) / oldMultiplier;
}

// Political Rally (National Event Card): counts down a closed square's
// remaining rounds and reopens it once the countdown reaches 0.
void reopenClosedSquares(GameState *gamestate) {

    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        if (square -> closedRoundsLeft > 0) {
            square -> closedRoundsLeft = square -> closedRoundsLeft - 1;

            if (square -> closedRoundsLeft == 0) {
                square -> isDamaged = 0;
            }
        }

        i = i + 1;
    }
}