#include "bookmgmt/Budget.h"

#include <cmath>
#include <iomanip>
#include <ostream>
#include <stdexcept>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

namespace {

const ResourceCategory kAllCategories[] = {
    ResourceCategory::Book,
    ResourceCategory::ElectronicResource,
    ResourceCategory::Journal,
    ResourceCategory::EBook,
    ResourceCategory::AudioBook,
    ResourceCategory::Thesis
};

bool isPrintCategory(ResourceCategory cat) {
    return cat == ResourceCategory::Book || 
           cat == ResourceCategory::Journal || 
           cat == ResourceCategory::Thesis;
}

}  // namespace

Budget::Budget(Money total) : total_(total) {
    if (total_.isNegative()) throw std::invalid_argument("budget must not be negative");
}

void Budget::setQuota(ResourceCategory c, Quota q) {
    if (q.maxUnits < 0 || q.maxSpend.isNegative())
        throw std::invalid_argument("quota limits must not be negative");
    quotas_[c] = q;
}

void Budget::removeQuota(ResourceCategory c) { quotas_.erase(c); }

std::optional<Quota> Budget::quotaFor(ResourceCategory c) const {
    auto it = quotas_.find(c);
    if (it == quotas_.end()) return std::nullopt;
    return it->second;
}

Usage Budget::usageFor(ResourceCategory c) const {
    auto it = usage_.find(c);
    return it == usage_.end() ? Usage{} : it->second;
}

std::optional<int> Budget::unitsRemaining(ResourceCategory c) const {
    auto q = quotaFor(c);
    if (!q) return std::nullopt;
    return q->maxUnits - usageFor(c).units;
}

std::optional<Money> Budget::spendRemaining(ResourceCategory c) const {
    auto q = quotaFor(c);
    if (!q) return std::nullopt;
    return q->maxSpend - usageFor(c).spent;
}

Budget::Failure Budget::evaluate(ResourceCategory c, int units, Money baseCost,
                                 std::string& why) const {
    if (units <= 0) {
        why = "quantity must be positive";
        return Failure::BadInput;
    }
    if (baseCost.isNegative()) {
        why = "cost must not be negative";
        return Failure::BadInput;
    }

    // Apply post-tax cost calculation for evaluation
    Money cost = costWithTax(c, baseCost);

    if (auto left = unitsRemaining(c); left && units > *left) {
        why = std::string(categoryName(c)) + " unit quota exceeded: requested " +
              std::to_string(units) + ", " + std::to_string(*left) + " remaining";
        return Failure::Quota;
    }
    if (auto left = spendRemaining(c); left && cost > *left) {
        why = std::string(categoryName(c)) + " spend quota exceeded: cost " +
              cost.toString() + ", " + left->toString() + " remaining";
        return Failure::Quota;
    }
    if (cost > remaining()) {
        why = "overall budget exceeded: cost " + cost.toString() + ", " +
              remaining().toString() + " remaining";
        return Failure::Overall;
    }
    why.clear();
    return Failure::None;
}

std::string Budget::check(ResourceCategory c, int units, Money cost) const {
    std::string why;
    evaluate(c, units, cost, why);
    return why;
}

void Budget::commit(ResourceCategory c, int units, Money baseCost) {
    std::string why;
    switch (evaluate(c, units, baseCost, why)) {
        case Failure::None: break;
        case Failure::BadInput: throw std::invalid_argument(why);
        case Failure::Quota: throw QuotaExceededError(why);
        case Failure::Overall: throw BudgetExceededError(why);
    }
    
    // Deduct total post-tax cost from budget
    Money totalCost = costWithTax(c, baseCost);
    Usage& u = usage_[c];
    u.units += units;
    u.spent += totalCost;
    spent_ += totalCost;
}

void Budget::print(std::ostream& os) const {
    os << "Budget: total " << total_ << ", spent " << spent_ << ", remaining "
       << remaining() << "\n";
    os << std::left << std::setw(22) << "  Category" << std::setw(18) << "Units used/max"
       << "Spend used/max\n";
    for (ResourceCategory c : kAllCategories) {
        const Usage u = usageFor(c);
        const auto q = quotaFor(c);
        const std::string units =
            std::to_string(u.units) + "/" + (q ? std::to_string(q->maxUnits) : "-");
        const std::string spend =
            u.spent.toString() + "/" + (q ? q->maxSpend.toString() : "-");
        os << "  " << std::setw(20) << categoryName(c) << std::setw(18) << units << spend
           << "\n";
    }
}

// ==========================================
// Question 6: Tax Calculations Implementation
// ==========================================

Money Budget::calculateTax(ResourceCategory cat, Money baseCost) const {
    double rate = isPrintCategory(cat) ? printTaxRate_ : electronicTaxRate_;
    // baseCost.minorUnits() gives total minor units (paise/cents)
    std::int64_t taxMinor = static_cast<std::int64_t>(std::round(baseCost.minorUnits() * rate));
    return Money::fromMinor(taxMinor);
}

Money Budget::costWithTax(ResourceCategory cat, Money baseCost) const {
    return baseCost + calculateTax(cat, baseCost);
}

}  // namespace bookmgmt