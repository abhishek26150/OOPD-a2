#pragma once

#include <map>
#include <optional>
#include <ostream>
#include <string>

#include "bookmgmt/Money.h"
#include "bookmgmt/Resource.h"

namespace bookmgmt {

struct Quota {
    int maxUnits = -1;       // -1 means no limit
    Money maxSpend = Money::of(-1); // negative means no limit
};

struct Usage {
    int units = 0;
    Money spent = Money::of(0);
};

class Budget {
public:
    explicit Budget(Money total);

    Money total() const { return total_; }
    Money spent() const { return spent_; }
    Money remaining() const { return total_ - spent_; }

    void setQuota(ResourceCategory c, Quota q);
    void removeQuota(ResourceCategory c);
    std::optional<Quota> quotaFor(ResourceCategory c) const;

    Usage usageFor(ResourceCategory c) const;
    std::optional<int> unitsRemaining(ResourceCategory c) const;
    std::optional<Money> spendRemaining(ResourceCategory c) const;

    enum class Failure { None, BadInput, Quota, Overall };

    Failure evaluate(ResourceCategory c, int units, Money cost,
                     std::string& why) const;

    std::string check(ResourceCategory c, int units, Money cost) const;
    void commit(ResourceCategory c, int units, Money cost);

    void print(std::ostream& os) const;

    // Question 6: Tax configuration & helper methods
    void setPrintTaxRate(double rate) { printTaxRate_ = rate; }
    void setElectronicTaxRate(double rate) { electronicTaxRate_ = rate; }

    double printTaxRate() const { return printTaxRate_; }
    double electronicTaxRate() const { return electronicTaxRate_; }

    Money calculateTax(ResourceCategory cat, Money baseCost) const;
    Money costWithTax(ResourceCategory cat, Money baseCost) const;

private:
    Money total_;
    Money spent_ = Money::of(0);
    std::map<ResourceCategory, Quota> quotas_;
    std::map<ResourceCategory, Usage> usage_;

    // Tax rates default to 0.0 (0%)
    double printTaxRate_ = 0.0;
    double electronicTaxRate_ = 0.0;
};

}  // namespace bookmgmt