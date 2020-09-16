#pragma once
#include <stdint.h>


struct timeval;
namespace Alime 
{
	/*
A Duration represents the elapsed time between two instantsas an int64
nanosecond count. The representation limits the largest representable duration to approximately 290 years.
This class is modified from Evpp
*/
	class  Duration 
	{
	public:
		static const int64_t kNanosecond = 1LL;
		static const int64_t kMicrosecond = 1000 * kNanosecond;
		static const int64_t kMillisecond = 1000 * kMicrosecond;
		static const int64_t kSecond = 1000 * kMillisecond;
		static const int64_t kMinute = 60 * kSecond;
		static const int64_t kHour = 60 * kMinute;

		static Duration Seconds(double count);
		static Duration Milliseconds(double count);
		static Duration Microseconds(double count);
		static Duration Minutes(double count);
		static Duration Hours(double count);
		static Duration Nanoseconds(int64_t count);
	public:
		Duration();
		explicit Duration(const struct timeval& t);
		explicit Duration(int64_t nanoseconds);
		explicit Duration(int nanoseconds);
		explicit Duration(double seconds);

		// Nanoseconds returns the duration as an integer nanosecond count.
		int64_t toNanoseconds() const;
		double toSeconds() const;
		double toMilliseconds() const;
		double toMicroseconds() const;
		double toMinutes() const;
		double toHours() const;

		struct timeval TimeVal() const;
		void To(struct timeval* t) const;
		bool IsZero() const;
		bool Valid() const;

		bool operator< (const Duration& rhs) const;
		bool operator<=(const Duration& rhs) const;
		bool operator> (const Duration& rhs) const;
		bool operator>=(const Duration& rhs) const;
		bool operator==(const Duration& rhs) const;

		Duration operator+=(const Duration& rhs);
		Duration operator-=(const Duration& rhs);
		Duration operator*=(int ns);
		Duration operator/=(int ns);

	private:
		int64_t ns_; // nanoseconds
	};
} // namespace end

