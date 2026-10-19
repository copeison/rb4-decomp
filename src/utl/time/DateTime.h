#pragma once

// A calendar date and time (utl/DateTime.o). Only the layout and the clock
// read are declared; the object is not reconstructed. The field names are
// not in the reference map; the map's constructor takes them as (year,
// month, day, hour, minute, second).
class DateTime {
public:
    unsigned char mSec;
    unsigned char mMin;
    unsigned char mHour;
    unsigned char mDay;
    // Zero-based.
    unsigned char mMonth;
    // Years since 1900.
    unsigned char mYear;
};

static_assert(sizeof(DateTime) == 6);

// The local date and time from the real-time clock. The map has
// GetDateAndTime(DateTime&); this build returns the value.
DateTime GetDateAndTime();  // 0x242700
