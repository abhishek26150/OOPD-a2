// Name: Abhishek Kumar Singh
// Roll Number: MT26150



#pragma once

#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <string>

#include "bookmgmt/Money.h"
#include "bookmgmt/Resource.h"

namespace bookmgmt {

struct Quota {
    int maxUnits = -1;       // -1 means no limit
    Money maxSpend = Money::of(-1); // negative means no limit
    int maxTitles = -1;      // -1 means no limit (Question 7)
};

struct Usage {
    int units = 0;
    Money spent = Money::of(0);
    std::set<std::string> purchasedResourceIds; // Question 7: Track distinct title IDs
    int distinctTitles() const { return static_cast<int>(purchasedResourceIds.size()); }
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
    
    // Question 7: Titles remaining helper
    std::optional<int> titlesRemaining(ResourceCategory c) const;

    enum class Failure { None, BadInput, Quota, Overall };

    Failure evaluate(ResourceCategory c, int units, Money cost,
                     std::string& why) const;
                     
    // Overload/Update to evaluate title limit
    Failure evaluate(ResourceCategory c, int units, Money cost,
                     const std::string& resourceId, std::string& why) const;

    std::string check(ResourceCategory c, int units, Money cost, const std::string& resourceId = "") const;
    void commit(ResourceCategory c, int units, Money cost, const std::string& resourceId = "");

    void print(std::ostream& os) const;

    // Question 6: Tax configuration
    void setPrintTaxRate(double rate) { printTaxRate_ = rate; }
    void setElectronicTaxRate(double rate) { electronicTaxRate_ = rate; }

    double printTaxRate() const { return printTaxRate_; }
    double electronicTaxRate() const { return electronicTaxRate_; }

    Money calculateTax(ResourceCategory cat, Money baseCost) const;
    Money costWithTax(ResourceCategory cat, Money baseCost) const;
    
    void refund(ResourceCategory c, int units, Money cost, const std::string& resourceId = "");

    // Question 10: Create next year's budget with unspent rollover carryover %
    static Budget createRollover(const Budget& currentBudget, double carryForwardPercent);

private:
    Money total_;
    Money spent_ = Money::of(0);
    std::map<ResourceCategory, Quota> quotas_;
    std::map<ResourceCategory, Usage> usage_;

    double printTaxRate_ = 0.0;
    double electronicTaxRate_ = 0.0;
};

}  // namespace bookmgmt