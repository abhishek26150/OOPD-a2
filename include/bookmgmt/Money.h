#pragma once
// Money: fixed-point currency amount stored in minor units (e.g. paise/cents) with currency code support.

#include <cstdint>
#include <iosfwd>
#include <string>
#include <stdexcept>

namespace bookmgmt {

class Money {
public:
    Money() : minor_(0), currency_("USD") {}

    // Construct from minor units: Money::fromMinor(12550, "USD") == 125.50 USD
    static Money fromMinor(std::int64_t minor, std::string currency = "USD") {
        return Money(minor, currency);
    }

    // Construct from major + minor parts: Money::of(125, 50, "USD") == 125.50 USD
    static Money of(std::int64_t major, int minor = 0, std::string currency = "USD");

    std::int64_t minorUnits() const { return minor_; }
    const std::string& currency() const { return currency_; }
    double toDouble() const { return static_cast<double>(minor_) / 100.0; }
    std::string toString() const;  // "1234.50" / "-3.05"

    bool isZero() const { return minor_ == 0; }
    bool isNegative() const { return minor_ < 0; }

    Money& operator+=(const Money& o) {
        checkCurrency(o);
        minor_ += o.minor_;
        return *this;
    }

    Money& operator-=(const Money& o) {
        checkCurrency(o);
        minor_ -= o.minor_;
        return *this;
    }

    Money& operator*=(std::int64_t k) {
        minor_ *= k;
        return *this;
    }

    friend Money operator+(Money a, const Money& b) { return a += b; }
    friend Money operator-(Money a, const Money& b) { return a -= b; }
    friend Money operator*(Money a, std::int64_t k) { return a *= k; }
    friend Money operator*(std::int64_t k, Money a) { return a *= k; }

    friend bool operator==(const Money& a, const Money& b) {
        a.checkCurrency(b);
        return a.minor_ == b.minor_;
    }
    friend bool operator!=(const Money& a, const Money& b) { return !(a == b); }
    
    friend bool operator<(const Money& a, const Money& b) {
        a.checkCurrency(b);
        return a.minor_ < b.minor_;
    }
    friend bool operator<=(const Money& a, const Money& b) { return a < b || a == b; }
    friend bool operator>(const Money& a, const Money& b) { return !(a <= b); }
    friend bool operator>=(const Money& a, const Money& b) { return !(a < b); }

private:
    explicit Money(std::int64_t minor, std::string currency = "USD")
        : minor_(minor), currency_(std::move(currency)) {}

    void checkCurrency(const Money& o) const {
        if (currency_ != o.currency_) {
            throw std::invalid_argument("Currency mismatch: " + currency_ + " vs " + o.currency_);
        }
    }

    std::int64_t minor_ = 0;
    std::string currency_ = "USD";
};

std::ostream& operator<<(std::ostream& os, Money m);

}  // namespace bookmgmt