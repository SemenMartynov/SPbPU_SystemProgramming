// Language
// Tategories
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: OVERFLOW_CATEGORY
//
// MessageText:
//
// An overflow exception category.
//
#define OVERFLOW_CATEGORY                ((WORD)0x00000001L)

//
// MessageId: ZERODIVIDE_CATEGORY
//
// MessageText:
//
// A division by zero exception category.
//
#define ZERODIVIDE_CATEGORY              ((WORD)0x00000002L)

// Determiners
//
// MessageId: READY_FOR_EXCEPTION
//
// MessageText:
//
// Ready for generate exception.
//
#define READY_FOR_EXCEPTION              ((DWORD)0x00000100L)

//
// MessageId: CAUGHT_EXCEPRION
//
// MessageText:
//
// Exclusive event happened.
//
#define CAUGHT_EXCEPRION                 ((DWORD)0x00000101L)

