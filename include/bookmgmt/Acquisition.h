#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "bookmgmt/Budget.h"
#include "bookmgmt/Catalog.h"

namespace bookmgmt {

struct PurchaseRequest {
    std::string resourceId;
    int quantity;
};

struct PurchaseRecord {
    int orderNo;
    std::string resourceId;
    std::string title;
    ResourceCategory category;
    int quantity;
    Money cost;
    bool approved;
    std::string reason;  // why it was rejected; empty if approved
    bool isCancellation = false; // Question 8: Cancellation record flag
};

class AcquisitionManager {
public:
    AcquisitionManager(Catalog& catalog, Budget& budget);

    Money quote(const std::string& id, int quantity) const;

    bool canPurchase(const std::string& id, int quantity,
                     std::string* reason = nullptr) const;

    const PurchaseRecord& purchase(const std::string& id, int quantity);

    std::vector<PurchaseRecord> processBatch(const std::vector<PurchaseRequest>& reqs);

    // Question 8: Order cancellation
    PurchaseRecord cancelOrder(int orderNo);

    const std::vector<PurchaseRecord>& history() const { return history_; }
    Money totalSpent() const;

    void printReport(std::ostream& os) const;

private:
    PurchaseRecord& record(const Resource* r, const std::string& id, int qty,
                           Money cost, bool approved, std::string reason);

    Catalog& catalog_;
    Budget& budget_;
    std::vector<PurchaseRecord> history_;
    int nextOrderNo_ = 1;
};

}  // namespace bookmgmt