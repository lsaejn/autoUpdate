#include "Duration.h"
#include <WinSock2.h>

namespace Alime
{
	Duration::Duration()
		: ns_(0) {}

	Duration::Duration(const struct timeval& t)
		: ns_(t.tv_sec * kSecond + t.tv_usec * kMicrosecond) {}

	Duration::Duration(int64_t nanoseconds)
		: ns_(nanoseconds) {}

	Duration::Duration(int nanoseconds)
		: ns_(nanoseconds) {}

	Duration::Duration(double seconds)
		: ns_(static_cast<int64_t>(seconds * kSecond)) {}

	int64_t Duration::toNanoseconds() const {
		return ns_;
	}

	double Duration::toSeconds() const {
		return double(ns_) / kSecond;
	}

	double Duration::toMilliseconds() const {
		return double(ns_) / kMillisecond;
	}

	double Duration::toMicroseconds() const {
		return double(ns_) / kMicrosecond;
	}

	double Duration::toMinutes() const {
		return double(ns_) / kMinute;
	}

	double Duration::toHours() const {
		return double(ns_) / kHour;
	}

	bool Duration::IsZero() const {
		return ns_ == 0;
	}

	bool Duration::Valid() const
	{
		return ns_ >= 0;
	}

	struct timeval Duration::TimeVal() const {
		struct timeval t;
		To(&t);
		return t;
	}

	void Duration::To(struct timeval* t) const {
		t->tv_sec = static_cast<long>(ns_ / kSecond);
		t->tv_usec = static_cast<long>(ns_ % kSecond) / static_cast<long>(kMicrosecond);
	}

	bool Duration::operator<(const Duration& rhs) const {
		return ns_ < rhs.ns_;
	}

	bool Duration::operator<=(const Duration& rhs) const {
		return ns_ <= rhs.ns_;
	}

	bool Duration::operator>(const Duration& rhs) const {
		return ns_ > rhs.ns_;
	}

	bool Duration::operator>=(const Duration& rhs) const {
		return ns_ >= rhs.ns_;
	}

	bool Duration::operator==(const Duration& rhs) const {
		return ns_ == rhs.ns_;
	}

	Duration Duration::operator+=(const Duration& rhs) {
		ns_ += rhs.ns_;
		return *this;
	}

	Duration Duration::operator-=(const Duration& rhs) {
		ns_ -= rhs.ns_;
		return *this;
	}

	Duration Duration::operator*=(int n) {
		ns_ *= n;
		return *this;
	}

	Duration Duration::operator/=(int n) {
		ns_ /= n;
		return *this;
	}

	Duration Duration::Seconds(double count)
	{
		return Duration(static_cast<int64_t>(count*Duration::kSecond));
	}

	Duration Duration::Milliseconds(double count)
	{
		return Duration(static_cast<int64_t>(count*Duration::kMillisecond));
	}

	Duration Duration::Microseconds(double count)
	{
		return Duration(static_cast<int64_t>(count*Duration::kMicrosecond));
	}

	Duration Duration::Minutes(double count)
	{
		return Duration(static_cast<int64_t>(count*Duration::kMinute));
	}

	Duration Duration::Hours(double count)
	{
		return Duration(static_cast<int64_t>(count*Duration::kHour));
	}

	Duration Duration::Nanoseconds(int64_t count)
	{
		return Duration(count);
	}
} // namespace end
