#include "bookmgmt/Acquisition.h"

#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdexcept>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

AcquisitionManager::AcquisitionManager(Catalog& catalog, Budget& budget)
    : catalog_(catalog), budget_(budget) {}

Money AcquisitionManager::quote(const std::string& id, int quantity) const {
    return catalog_.get(id).costFor(quantity);
}

bool AcquisitionManager::canPurchase(const std::string& id, int quantity,
                                     std::string* reason) const {
    std::string why;
    if (const Resource* r = catalog_.find(id)) {
        if (quantity <= 0)
            why = "quantity must be positive";
        else
            why = budget_.check(r->category(), quantity, r->costFor(quantity), id);
    } else {
        why = "resource not found: " + id;
    }
    if (reason) *reason = why;
    return why.empty();
}

PurchaseRecord& AcquisitionManager::record(const Resource* r, const std::string& id,
                                           int qty, Money cost, bool approved,
                                           std::string reason) {
    history_.push_back(PurchaseRecord{
        nextOrderNo_++, id, r ? r->title() : std::string("(unknown)"),
        r ? r->category() : ResourceCategory::Book, qty, cost, approved,
        std::move(reason)});
    return history_.back();
}

const PurchaseRecord& AcquisitionManager::purchase(const std::string& id, int quantity) {
    const Resource& r = catalog_.get(id);        // may throw NotFoundError
    const Money cost = r.costFor(quantity);      // may throw invalid_argument
    budget_.commit(r.category(), quantity, cost, id);  // may throw quota/budget errors
    catalog_.addHoldings(id, quantity);
    return record(&r, id, quantity, cost, true, {});
}

std::vector<PurchaseRecord> AcquisitionManager::processBatch(
    const std::vector<PurchaseRequest>& reqs) {
    std::vector<PurchaseRecord> results;
    results.reserve(reqs.size());
    for (const auto& req : reqs) {
        const Resource* r = catalog_.find(req.resourceId);
        Money cost;
        std::string why;
        if (!r) {
            why = "resource not found: " + req.resourceId;
        } else if (req.quantity <= 0) {
            why = "quantity must be positive";
        } else {
            cost = r->costFor(req.quantity);
            why = budget_.check(r->category(), req.quantity, cost, req.resourceId);
        }

        if (why.empty()) {
            results.push_back(purchase(req.resourceId, req.quantity));
        } else {
            results.push_back(record(r, req.resourceId, req.quantity, cost, false, why));
        }
    }
    return results;
}

// Question 8: Order Cancellation
PurchaseRecord AcquisitionManager::cancelOrder(int orderNo) {
    // 1. Locate original order in history
    PurchaseRecord* orig = nullptr;
    for (auto& rec : history_) {
        if (rec.orderNo == orderNo) {
            orig = &rec;
            break;
        }
    }

    if (!orig) {
        throw NotFoundError("Order #" + std::to_string(orderNo) + " not found");
    }

    if (!orig->approved) {
        throw std::invalid_argument("Cannot cancel a rejected order #" + std::to_string(orderNo));
    }

    if (orig->isCancellation) {
        throw std::invalid_argument("Cannot cancel a cancellation record");
    }

    // Check if order was already cancelled
    for (const auto& rec : history_) {
        if (rec.isCancellation && rec.reason == "Cancelled #" + std::to_string(orderNo)) {
            throw std::invalid_argument("Order #" + std::to_string(orderNo) + " is already cancelled");
        }
    }

    // 2. Reduce holdings in catalog
    catalog_.addHoldings(orig->resourceId, -orig->quantity);

    // 3. Refund budget and quota
    budget_.refund(orig->category, orig->quantity, orig->cost, orig->resourceId);

    // 4. Record cancellation entry in history
    PurchaseRecord cancelRec;
    cancelRec.orderNo = nextOrderNo_++;
    cancelRec.resourceId = orig->resourceId;
    cancelRec.category = orig->category;
    cancelRec.title = orig->title;
    cancelRec.quantity = -orig->quantity;
    cancelRec.cost = Money::of(0) - orig->cost;
    cancelRec.approved = true;

    std::ostringstream ss;
    ss << "Cancelled #" << orderNo;
    cancelRec.reason = ss.str();
    cancelRec.isCancellation = true;

    history_.push_back(cancelRec);
    return cancelRec;
}

Money AcquisitionManager::totalSpent() const {
    Money sum;
    for (const auto& rec : history_)
        if (rec.approved) sum += rec.cost;
    return sum;
}

void AcquisitionManager::printReport(std::ostream& os) const {
    os << "Order history (" << history_.size() << " orders)\n";
    for (const auto& rec : history_) {
        os << "  #" << std::setw(3) << std::left << rec.orderNo << " "
           << (rec.approved ? "APPROVED" : "REJECTED") << "  " << std::setw(6)
           << rec.resourceId << " x" << std::setw(3) << rec.quantity << " "
           << std::setw(12) << std::right << rec.cost.toString() << std::left << "  "
           << rec.title;
        if (!rec.approved) os << "\n        reason: " << rec.reason;
        if (rec.isCancellation) os << " (" << rec.reason << ")";
        os << "\n";
    }
    os << "Total spent: " << totalSpent() << "\n";
}

}  // namespace bookmgmt