#pragma once

#include "duration.h"
#include <string>


namespace Alime 
{
	class Timestamp 
	{
	public:
		Timestamp();
		explicit Timestamp(int64_t nanoseconds); //rarely used
		explicit Timestamp(const struct timeval& t);

		static Timestamp Now(); // returns the current local time.

		struct timeval TimeVal() const;
		void To(struct timeval* t) const;

		// Unix returns t as a Unix time, the number of seconds elapsed since January 1, 1970 UTC.
		int64_t Unix() const;

		// the number of nanoseconds/microseconds elapsed since January 1, 1970 UTC. The result is undefined
		// if the Unix time in nanoseconds/microsecond cannot be represented by an int64.
		int64_t UnixNano() const;
		int64_t UnixMicro() const;

		void Add(Duration d);

		bool IsEpoch() const;
		bool operator< (const Timestamp& rhs) const;
		bool operator==(const Timestamp& rhs) const;

		Timestamp operator+=(const Duration& rhs);
		Timestamp operator+ (const Duration& rhs) const;
		Timestamp operator-=(const Duration& rhs);
		Timestamp operator- (const Duration& rhs) const;
		Duration  operator- (const Timestamp& rhs) const;

		bool valid() const;
		static Timestamp invalid();
		std::string toString() const;// 以字符形式打印ticks
		std::string toFormattedString(bool showMicroseconds = true) const;
	private:
		// ns_ gives the number of nanoseconds elapsed since the Epoch
		// 1970-01-01 00:00:00 +0000 (UTC).
		int64_t ns_;
	};
}
