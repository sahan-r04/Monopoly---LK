#ifndef TYPES_H
#define TYPES_H

#define BOARD_SIZE                              40
#define NUM_PLAYERS                              4
#define STARTING_CASH                        30000
#define MAX_ROUNDS                             500
#define PASSING_GO_PAY                        2000
#define BAIL_AMOUNT                            300
#define MAX_NAME_LEN                            50
#define AUCTION_INCREMENT                      250
#define MAX_JAIL_TURNS                           3
#define LOAN_DURATION_ROUNDS                    20
#define INSURANCE_DURATION_ROUNDS               20
#define INSURANCE_WARNING_ROUNDS                 3
#define MAX_HOUSES                               4
#define DEPRECIATION_AGE_LIMIT                  50
#define DEPRECIATION_MAX_PERCENTAGE             30
#define MAINTENANCE_NEGLECT_LIMIT               20
#define BUILDING_DECAY_PERCENTAGE_PER_ROUND      2
#define MARKET_BOOM_COOLDOWN_RONDS              30
#define NATIONAL_CARD_DURATION_ROUNDS           15
#define REGIONAL_CARD_DURATION_ROUNDS           15
#define GOVERNMENT_REGULATION_DURATION_ROUNDS   20
#define INCOME_TAX_AMOUNT                      200
#define GROUP_COOLDOWN_ROUNDS                   30

typedef enum SquareType{
    SQUARE_START,
    SQUARE_PROPERTY,
    SQUARE_RAILWAY,
    SQUARE_UTILITY,
    SQUARE_TAX,
    SQUARE_EVENT,
    SQUARE_JAIL,
    SQUARE_FREE_PARKING,
    SQUARE_GO_TO_JAIL,
    SQUARE_INSURANCE,
    SQUARE_BANK,
} SquareType;

typedef enum PropertyGroup {
    GROUP_NONE, // Square isn't a property
    GROUP_BROWN, // Group 1 (Pettah, Maradana)
    GROUP_LIGHT_BLUE, // Group 2 (Bambalapitiya, Wellawatte, Mount Lavinia)
    GROUP_PINK, // Group 3 (Maharagama, Nugegoda, Kottawa)
    GROUP_ORANGE, // Group 4 (Negombo, Katunayake, Ja-Ela)
    GROUP_RED, // Group 5 (Kandy City, Peradeniya, Katugastota)
    GROUP_YELLOW, //Group 6 (Galle Fort, nawatuna, Hikkaduwa)
    GROUP_GREEN, //Group 7 (Jaffna Town, Nallur, Trincomalee)
    GROUP_DARK_BLUE, //Group 8 (Nuwara Eliya, Galle Face)
} PropertyGroup;

typedef enum StrategyType {
    STRATEGY_AGGRESSIVE, // Aggressive Investor
    STRATEGY_CONSERVATIVE, // Conservative Banker
    STRATEGY_RISK_TAKER, // Risk Taker
    STRATEGY_OPPORTUNISTIC, // Opportunistic Trader
} StrategyType;

typedef enum InsuranceType {
    INSURANCE_NONE = -1,
    INSURANCE_BASIC,
    INSURANCE_COMPREHENSIVE,
    INSURANCE_BUSINESS_INTERRUPTION,
} InsuranceType;

typedef enum DisasterType {
    DISASTER_FIRE,
    DISASTER_FLOOD,
    DISASTER_RIOT,
    DISASTER_COLLAPSE,
    DISASTER_ELECTRICAL,
} DisasterType;

typedef struct Square {
    int squareIndex; // 0-39 position on the board 
    char name[MAX_NAME_LEN]; // Name of the square
    SquareType type;
    PropertyGroup group; //GROUP_NONE if isn't a property

    //----------Static Values--------//
    int purchasePrice;    
    int baseRent;
    int houseCost;
    int hotelCost;
    int mortgageValue;

    //----------Dynamic Values--------//
    int currentValue;
    int currentRent;

    //------Property Ownership Details-----//
    int ownerId;
    int isMortgaged; // 0 or 1
    int isLoanLocked; // 0 or 1

    //----Development (Rule 8,9,10)----//
    int numHouses;
    int hasHotel;

    //----Condition & age----//
    int buildingCondition;
    int propertyAge;
    int depreciationPercent; // running total, capped at DEPRECIATION_MAX_PERCENTAGE
    int isDamaged;
    int closedRoundsLeft; // Political Rally: rounds left before this square reopens

    //----Insurance----//
    InsuranceType insurancePolicy; //None, Basic, Comprehensive, Business Interruption
    int insuranceRoundsLeft;
} Square;

typedef struct ActiveEffect{
    int isActive; //0 or 1
    int effectId;
    int cardId; // which card/regulation this is, so its name can be looked
                // up again later for display (Rule-LK 36) or for a specific check
    PropertyGroup targetGroup;
    int roundsRemaining; //counting remaining rounds get effected
} ActiveEffect;

typedef struct Player {
    char name[MAX_NAME_LEN];
    StrategyType strategy;

    int cash;
    int position;
    int lapsCompleted; // times passed/landed on GO - drives round counting (game.c)
    int inJail;
    int jailTurns; // counts 0 to 3 while in jail
    int isBankrupt;

    //-----Each player may have at most 1 active loan----//
    int hasLoan; // 0 or 1
    int loanAmount; // outstanding balance
    int loanInterestRate; 
    int loanRoundsLeft;
    int loanCollateral[BOARD_SIZE]; // square(property/utility) pledged to this loan

    //-----Active National Event Card effect on this player (Appendix A)-----//
    // effectId holds the drawn card's rentMultiplierPercentage while this is active.
    ActiveEffect nationalCard;
} Player;

typedef struct Economy{
    int round;
    int inflationRate;
    int baseInterestRate;

    //-----Active Regional Development Card effect (Section 2.10)-----//
    // effectId holds the drawn card's valuePercentageChange while this is active.
    ActiveEffect regionalCard;

    //-----Active Government Regulation effect (Section 2.7)-----//
    // effectId holds the index (0-7) of the active regulation in
    // governmentRegulationNames (events.c).
    ActiveEffect governmentRegulation;
    ActiveEffect marketBoom;
    ActiveEffect marketDecline;

    //----Rule 33-avoid repeating the same group----//
    PropertyGroup lastBoomGroup;
    PropertyGroup lastDeclineGroup;
    int groupCooldownRounds[9]; // index 1-8 (PropertyGroup); rounds left before
                                 // that group can be picked for a boom/decline again

    //----Fields the various cards/regulations write into (Rule-LK 13/24)----//
    int currentIncomeTaxAmount;        // starts at INCOME_TAX_AMOUNT (Rule 11)
    int insurancePremiumDiscountPercent; // Insurance Discount / Insurance Regulation
    int constructionSuspendedRoundsLeft; // Labour Strike
    int antiSpeculationActive;          // Anti-Speculation Act (0 or 1)
} Economy;

typedef struct NationalEventCard {
    int cardId;              // fixed ID (0-19) that stays with the card as it moves
                              // through the deck, so it can still be identified after
                              // being sent to the bottom
    char name[40];
    int cashBonus;          // 0 if no direct cash effect
    int rentMultiplierPercentage;  // 100% = normal, 200% = double, 0 = not applicabl
} NationalEventCard;

typedef struct RegionalDevelopmentCard {
    int cardId;              // fixed ID (0-11), same purpose as NationalEventCard.cardId
    char name[50];
    int valuePercentageChange;   // +25 = property values in the targetted group rise 25%
} RegionalDevelopmentCard;

typedef struct EconomicEventCard {
    int cardId;              // fixed ID (0-7), same purpose as the other two card types
    char name[50];
} EconomicEventCard;

typedef struct GameState{
    Square board[BOARD_SIZE];
    Player players[NUM_PLAYERS];
    Economy economy;
} GameState;

//===========================================================================
// Function prototypes for every function that's called from a .c file other
// than the one that defines it. Grouped by the file that actually defines
// each function, so every other file can just #include "types.h" and call
// them without writing its own duplicate forward declaration.
//===========================================================================

// ---- board.c ----
void monopolyBoard(Square board[BOARD_SIZE]);
int movePlayer(Player *p, int diceTotal);
void sendToJail(Player *p);
int getRentMultiplier(Square *square);
void recalculateRentAfterConstruction(Square *square, int oldMultiplier, int newMultiplier);
void reopenClosedSquares(GameState *gamestate);

// ---- finance.c ----
int calculateMaxLoan(GameState *gamestate, int playerIndex);
void takeLoan(GameState *gamestate, int playerIndex, int amount, int collateralChoice[BOARD_SIZE]);
void repayLoan(GameState *gamestate, int playerIndex, int amount);
void applyLoanInterest(GameState *gamestate);
void getInsurance(GameState *gamestate, int playerIndex, int squareIndex, InsuranceType policy);
void payRent(GameState *gamestate, int playerIndex, int squareIndex);
int calculateRailwayRent(GameState *gamestate, int squareIndex);
int calculateUtilityRent(GameState *gamestate, int squareIndex, int diceTotal);
void collectTax(GameState *gamestate, int playerIndex);
int calculateNetWorth(GameState *gamestate, int playerIndex);
void checkBankruptcy(GameState *gamestate);
void retryDamageRepairs(GameState *gamestate);
void applyRoundTickEffects(GameState *gamestate);

// ---- events.c ----
void drawNationalEventCard(GameState *gamestate, int playerIndex);
void drawRegionalDevelopmentCard(GameState *gamestate);
void inflationChange(GameState *gamestate);
void propertyMarketChange(GameState *gamestate);
void governmentRegulationChange(GameState *gamestate);
void triggerDisaster(GameState *gamestate);
void drawEconomicEvent(GameState *gamestate);
void revertExpiredEffects(GameState *gamestate);
void displayMarketConditions(GameState *gamestate);

// ---- players.c ----
int monopolyPlay(GameState *gamestate, int playerIndex, PropertyGroup group);
int decideToPurchase(GameState *gamestate, int playerIndex, int squareIndex);
int decideAuctionBid(GameState *gamestate, int playerIndex, int squareIndex, int currentBid);
int decideLoanAmount(GameState *gamestate, int playerIndex);
int decideLoanRepaymentAmount(GameState *gamestate, int playerIndex);
int decideInsuranceTarget(GameState *gamestate, int playerIndex);
InsuranceType decideInsurancePolicy(GameState *gamestate, int playerIndex, int squareIndex);
int decideConstruction(GameState *gamestate, int playerIndex);
int decideMortgage(GameState *gamestate, int playerIndex);

// ---- game.c ----
void initializingPlayers(GameState *gamestate);
void determineTurnOrder(GameState *gamestate, int order[NUM_PLAYERS]);
void playTurn(GameState *gamestate, int playerIndex);
void runAuction(GameState *gamestate, int squareIndex);
void endOfRoundProcessing(GameState *gamestate);
void printRoundSummary(GameState *gamestate);
int isGameOver(GameState *gamestate);
int findWinner(GameState *gamestate);
void runGame(GameState *gamestate);

#endif