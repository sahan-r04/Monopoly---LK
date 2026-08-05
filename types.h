#ifndef TYPES_H
#define TYPES_H

#define BOARD_SIZE                          40
#define NUM_PLAYERS                          4
#define STARTING_CASH                    30000
#define MAX_ROUNDS                         500
#define PASSING_GO_PAY                    2000
#define BAIL_AMOUNT                        300
#define MAX_NAME_LEN                        32
#define AUCTION_INCREMENT                  250
#define MAX_JAIL_TURNS                       3
#define LOAN_DURATION_ROUNDS                20
#define INSURANCE_DURATION_ROUNDS           20
#define INSURANCE_WARNING_ROUNDS             3
#define MAX_HOUSES                           4
#define DEPRECIATION_AGE_LIMIT              50
#define DEPRECIATION_MAX_PERCENTAGE         30
#define MAINTENANCE_NEGLECT_LIMIT           20
#define BUILDING_DECAY_PERCENTAGE_PER_ROUND  2
#define MARKET_BOOM_COOLDOWN_RONDS          30

typedef enum {
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

typedef enum {
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

typedef enum {
    STRATEGY_AGGRESSIVE, // Aggressive Investor
    STRATEGY_CONSERVATIVE, // Conservative Banker
    STRATEGY_RISK_TAKER, // Risk Taker
    STRATEGY_OPPORTUNISTIC, // Opportunistic Trader
} StrategyType;

typedef enum {
    INSURANCE_NONE = -1,
    INSURANCE_BASIC,
    INSURANCE_COMPREHENSIVE,
    INSURANCE_BUSINESS_INTERRUPTION,
} InsuranceType;

typedef enum {
    DISASTER_FIRE,
    DISASTER_FLOOD,
    DISASTER_RIOT,
    DISASTER_COLLAPSE,
    DISASTER_ELECTRICAL,
} DisasterType;

typedef struct {
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
    int isDamaged;

    //----Insurance----//
    InsuranceType insurancePolicy; //None, Basic, Comprehensive, Business Interruption
    int insuranceRoundsLeft;
} Square;

typedef struct {
    char name[MAX_NAME_LEN];
    StrategyType strategy;

    int cash;
    int position;
    int inJail;
    int jailTurns; // counts 0 to 3 while in jail
    int isBankrupt;

    //-----Each player may have at most 1 active loan----//
    int hasLoan; // 0 or 1
    int loanAmount; // outstanding balance
    int loanInterestRate; 
    int loanRoundsLeft;
    int loanCollateral[BOARD_SIZE]; // square(property/utility) pledged to this loan
} Player;

typedef struct {
    int isActive; //0 or 1
    int effectId;
    PropertyGroup targetGroup;
    int roundsRemaining; //counting remaining rounds get effected
} ActiveEffect;

typedef struct {
    int round;
    int inflationRate;
    int baseInterestRate;

    //----National Event Card----//
    int nationalCardNextIndex;

    ActiveEffect regionalCard;
    ActiveEffect govRegulation;
    ActiveEffect marketBoom;
    ActiveEffect marketDecline;

    //----Rule 33-avoid repeating the same group----//
    PropertyGroup lastBoomGroup;
    PropertyGroup lastDeclineGroup;
} Economy;

typedef struct {
    Square board[BOARD_SIZE];
    Player players[NUM_PLAYERS];
    Economy economy;
} GameState;

#endif