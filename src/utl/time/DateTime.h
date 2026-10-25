#pragma once

class BinStream;

// A calendar date and time (utl/DateTime.o). Only the layout, the clock read
// and the members the saved song data use are declared; the object is not
// reconstructed. The field names are not in the reference map; the map's
// constructor takes them as (year, month, day, hour, minute, second).
class DateTime {
public:
    // All fields zero. Inline: SongPersistentData's copy constructor
    // (0xD6A080) zeroes the six bytes.
    DateTime() : mSec(0), mMin(0), mHour(0), mDay(0), mMonth(0), mYear(0) {}
    // The date of a packed code, as ToCode makes it. The map has the same
    // signature.
    explicit DateTime(unsigned int code);  // 0x242780

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

// The six fields, one byte each.
BinStream& operator<<(BinStream& stream, const DateTime& date);  // 0x244210
BinStream& operator>>(BinStream& stream, DateTime& date);        // 0x2442E0
