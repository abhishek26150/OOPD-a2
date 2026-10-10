#include "bookmgmt/Budget.h"
#include "bookmgmt/Resource.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

// Helper to identify print categories for tax rate selection
static bool isPrintCategory(ResourceCategory cat) {
    return cat == ResourceCategory::Book || cat == ResourceCategory::Journal;
}

Budget::Budget(Money total) : total_(total) {
    if (total.isNegative()) {
        throw std::invalid_argument("Total budget cannot be negative");
    }
}

void Budget::setQuota(ResourceCategory c, Quota q) {
    quotas_[c] = q;
}

void Budget::removeQuota(ResourceCategory c) {
    quotas_.erase(c);
}

std::optional<Quota> Budget::quotaFor(ResourceCategory c) const {
    auto it = quotas_.find(c);
    if (it != quotas_.end()) {
        return it->second;
    }
    return std::nullopt;
}

Usage Budget::usageFor(ResourceCategory c) const {
    auto it = usage_.find(c);
    if (it != usage_.end()) {
        return it->second;
    }
    return Usage{};
}

std::optional<int> Budget::unitsRemaining(ResourceCategory c) const {
    auto q = quotaFor(c);
    if (!q || q->maxUnits < 0) return std::nullopt;
    int used = usageFor(c).units;
    return std::max(0, q->maxUnits - used);
}

std::optional<Money> Budget::spendRemaining(ResourceCategory c) const {
    auto q = quotaFor(c);
    if (!q || q->maxSpend.isNegative()) return std::nullopt;
    Money used = usageFor(c).spent;
    if (used >= q->maxSpend) return Money::of(0);
    return q->maxSpend - used;
}

std::optional<int> Budget::titlesRemaining(ResourceCategory c) const {
    auto q = quotaFor(c);
    if (!q || q->maxTitles < 0) return std::nullopt;
    int distinct = usageFor(c).distinctTitles();
    return std::max(0, q->maxTitles - distinct);
}

Budget::Failure Budget::evaluate(ResourceCategory c, int units, Money cost,
                                 std::string& why) const {
    return evaluate(c, units, cost, "", why);
}

Budget::Failure Budget::evaluate(ResourceCategory c, int units, Money cost,
                                 const std::string& resourceId, std::string& why) const {
    why.clear();
    if (units <= 0 || cost.isNegative()) {
        why = "Invalid units or negative cost";
        return Failure::BadInput;
    }

    if (spent_ + cost > total_) {
        std::ostringstream ss;
        ss << "Overall budget exceeded: cost " << cost << ", " << remaining() << " remaining";
        why = ss.str();
        return Failure::Overall;
    }

    auto q = quotaFor(c);
    if (q) {
        Usage u = usageFor(c);

        if (q->maxUnits >= 0 && u.units + units > q->maxUnits) {
            std::ostringstream ss;
            ss << categoryName(c) << " unit quota exceeded: requested " << units
               << ", " << (q->maxUnits - u.units) << " remaining";
            why = ss.str();
            return Failure::Quota;
        }

        if (!q->maxSpend.isNegative() && u.spent + cost > q->maxSpend) {
            std::ostringstream ss;
            ss << categoryName(c) << " spend quota exceeded: cost " << cost
               << ", " << (q->maxSpend - u.spent) << " remaining";
            why = ss.str();
            return Failure::Quota;
        }

        // Question 7: Check distinct title limit
        if (q->maxTitles >= 0 && !resourceId.empty()) {
            bool isNewTitle = u.purchasedResourceIds.find(resourceId) == u.purchasedResourceIds.end();
            if (isNewTitle && u.distinctTitles() >= q->maxTitles) {
                std::ostringstream ss;
                ss << categoryName(c) << " title quota exceeded: max " << q->maxTitles << " titles allowed";
                why = ss.str();
                return Failure::Quota;
            }
        }
    }

    return Failure::None;
}

std::string Budget::check(ResourceCategory c, int units, Money cost, const std::string& resourceId) const {
    std::string why;
    evaluate(c, units, cost, resourceId, why);
    return why;
}

void Budget::commit(ResourceCategory c, int units, Money cost, const std::string& resourceId) {
    std::string why;
    Failure f = evaluate(c, units, cost, resourceId, why);
    if (f == Failure::Quota) {
        throw QuotaExceededError(why);
    } else if (f == Failure::Overall) {
        throw BudgetExceededError(why);
    } else if (f == Failure::BadInput) {
        throw std::invalid_argument(why);
    }

    spent_ += cost;
    auto& u = usage_[c];
    u.units += units;
    u.spent += cost;
    if (!resourceId.empty()) {
        u.purchasedResourceIds.insert(resourceId);
    }
}

void Budget::print(std::ostream& os) const {
    os << "Budget: total " << total_ << ", spent " << spent_ << ", remaining " << remaining() << "\n";
    os << "  Category            Units used/max    Spend used/max\n";

    static const std::vector<ResourceCategory> categories = {
        ResourceCategory::Book,
        ResourceCategory::ElectronicResource,
        ResourceCategory::Journal,
        ResourceCategory::EBook,
        ResourceCategory::AudioBook,
        ResourceCategory::Thesis
    };

    for (ResourceCategory cat : categories) {
        Usage u = usageFor(cat);
        auto q = quotaFor(cat);

        std::string unitsStr = std::to_string(u.units) + "/";
        unitsStr += (q && q->maxUnits >= 0) ? std::to_string(q->maxUnits) : "-";

        std::string spendStr = u.spent.toString() + "/";
        spendStr += (q && !q->maxSpend.isNegative()) ? q->maxSpend.toString() : "-";

        os << "  " << std::left << std::setw(20) << categoryName(cat)
           << std::setw(18) << unitsStr
           << spendStr << "\n";
    }
}

// Question 6: Tax Calculations Implementation
Money Budget::calculateTax(ResourceCategory cat, Money baseCost) const {
    double rate = isPrintCategory(cat) ? printTaxRate_ : electronicTaxRate_;
    std::int64_t taxMinor = static_cast<std::int64_t>(std::round(baseCost.minorUnits() * rate));
    return Money::fromMinor(taxMinor);
}

Money Budget::costWithTax(ResourceCategory cat, Money baseCost) const {
    return baseCost + calculateTax(cat, baseCost);
}

}  // namespace bookmgmt