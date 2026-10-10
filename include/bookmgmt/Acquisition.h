#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bookmgmt/Budget.h"
#include "bookmgmt/Catalog.h"

namespace bookmgmt {

struct PurchaseRequest {
    std::string resourceId;
    int quantity;
    std::string department = ""; // Question 9: Department name
};

struct PurchaseRecord {
    int orderNo;
    std::string resourceId;
    std::string title;
    ResourceCategory category;
    int quantity;
    Money cost;
    bool approved;
    std::string reason;
    bool isCancellation = false;
    std::string department = ""; // Question 9: Department charged
};

class AcquisitionManager {
public:
    AcquisitionManager(Catalog& catalog, Budget& budget);

    // Question 9: Department budget management
    void addDepartmentBudget(const std::string& dept, std::shared_ptr<Budget> budget) {
        deptBudgets_[dept] = budget;
    }

    Budget* getDepartmentBudget(const std::string& dept) const {
        auto it = deptBudgets_.find(dept);
        if (it != deptBudgets_.end()) return it->second.get();
        return nullptr;
    }

    Money quote(const std::string& id, int quantity) const;

    bool canPurchase(const std::string& id, int quantity,
                     std::string* reason = nullptr) const;

    bool canPurchase(const std::string& id, int quantity, const std::string& dept,
                     std::string* reason = nullptr) const;

    const PurchaseRecord& purchase(const std::string& id, int quantity, const std::string& dept = "");

    std::vector<PurchaseRecord> processBatch(const std::vector<PurchaseRequest>& reqs, bool allOrNothing = false);
    PurchaseRecord cancelOrder(int orderNo);

    const std::vector<PurchaseRecord>& history() const { return history_; }
    Money totalSpent() const;

    void printReport(std::ostream& os) const;

private:
    PurchaseRecord& record(const Resource* r, const std::string& id, int qty,
                           Money cost, bool approved, std::string reason,
                           const std::string& dept = "");

    Catalog& catalog_;
    Budget& defaultBudget_;
    std::map<std::string, std::shared_ptr<Budget>> deptBudgets_; // Question 9
    std::vector<PurchaseRecord> history_;
    int nextOrderNo_ = 1;
};

}  // namespace bookmgmt