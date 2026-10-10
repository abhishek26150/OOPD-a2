// Name: Abhishek Kumar Singh
// Roll Number: MT26150

// Demo: builds a small catalog, sets a budget with per-category quotas,
// and runs a batch of purchase requests through the acquisition manager.

#include <iostream>

#include "bookmgmt/bookmgmt.h"

using namespace bookmgmt;

#include "bookmgmt/EBook.h"
#include "bookmgmt/AudioBook.h"
#include "bookmgmt/Thesis.h"


int main() {
    Catalog catalog;

    catalog.emplace<Book>("B001", "Clean Code", std::vector<std::string>{"Robert C. Martin"},
                          "978-0132350884", "Prentice Hall", 2008, Money::of(450));
    catalog.emplace<Book>("B002", "The C++ Programming Language",
                          std::vector<std::string>{"Bjarne Stroustrup"}, "978-0321563842",
                          "Addison-Wesley", 2013, Money::of(1200), 4, Binding::Hardcover);
    catalog.emplace<ElectronicResource>("R001", "IEEE Xplore Digital Library", "IEEE", 2026,
                                        Money::of(150), "https://ieeexplore.example",
                                        LicenseModel::AnnualSubscription, Money::of(2000));
    catalog.emplace<ElectronicResource>("R002", "MATLAB Campus Licence", "MathWorks", 2026,
                                        Money::of(400), "https://licensing.example/matlab",
                                        LicenseModel::Perpetual);
    
    // Q1 Extension: Journal
    catalog.emplace<Journal>("J001", "IEEE Software", "IEEE", 2023, Money::of(150), "0740-7459", 6, 2);

    std::cout << "=== Catalog ===\n";
    for (const Resource* r : catalog.all()) std::cout << r->summary() << "\n";

    std::cout << "\n=== Details of R001 ===\n" << catalog.get("R001");
    std::cout << "\n=== Details of J001 (Journal) ===\n" << catalog.get("J001");

    catalog.emplace<bookmgmt::EBook>("EB101", "Designing Data-Intensive Applications",
                       std::vector<std::string>{"Martin Kleppmann"},
                       "978-1449373320", "O'Reilly", 2017, Money::of(300),
                       "https://oreilly.example/ddia", bookmgmt::FileFormat::PDF, false);

    catalog.emplace<AudioBook>("AB201", "Design Patterns",
                           std::vector<std::string>{"Erich Gamma", "Richard Helm"},
                           "978-0201633610", "Pearson", 2020, Money::of(400),
                           "https://audio.example/dp", 480, "Derek Perkins");

catalog.emplace<Thesis>("TH301", "Scalable Consensus Algorithms", "Rohan Sharma",
                        "Prof. A. Gupta", "IIIT Delhi", "M.Tech", 2025);


    Budget budget(Money::of(20000));
    budget.setQuota(ResourceCategory::Book, {10, Money::of(8000)});
    budget.setQuota(ResourceCategory::ElectronicResource, {40, Money::of(12000)});
    budget.setQuota(ResourceCategory::Journal, {10, Money::of(5000)});

    AcquisitionManager acq(catalog, budget);

    std::cout << "\n=== Quotes ===\n";
    std::cout << "5 copies of B002  = " << acq.quote("B002", 5) << "\n";
    std::cout << "20 seats of R001  = " << acq.quote("R001", 20) << "  (incl. platform fee)\n";
    std::cout << "2 subs of J001    = " << acq.quote("J001", 2) << "  (2-year sub)\n";

    acq.processBatch({
        {"B001", 4},   // 1800  ok
        {"B002", 5},   // 6000  ok  -> book spend 7800
        {"B001", 1},   // 450   rejected: book spend quota (200 left)
        {"R001", 20},  // 5000  ok
        {"R002", 25},  // 10000 rejected: e-resource unit quota (20 seats left)
        {"R002", 15},  // 6000  ok  -> e-resource spend 11000
        {"R002", 5},   // 2000  rejected: e-resource spend quota (1000 left)
        {"J001", 2},   // 600   ok  -> journal spend 600
        {"X999", 1},   // rejected: unknown id
    });

    std::cout << "\n=== Acquisition report ===\n";
    acq.printReport(std::cout);

    std::cout << "\n=== Budget ===\n";
    budget.print(std::cout);

    std::cout << "\n=== Holdings ===\n";
    for (const Resource* r : catalog.all())
        std::cout << "  " << r->id() << ": " << catalog.holdings(r->id())
                  << (r->isDigital() ? " seats" : " copies") << "\n";

    // Direct purchase: errors are reported with exceptions
    std::cout << "\n=== Direct purchase that breaks a quota ===\n";
    try {
        acq.purchase("B002", 1);
    } catch (const QuotaExceededError& e) {
        std::cout << "QuotaExceededError: " << e.what() << "\n";
    } catch (const BudgetExceededError& e) {
        std::cout << "BudgetExceededError: " << e.what() << "\n";
    }

    std::cout << "\n=== Export CSV (Question 5) ===\n";
    catalog.exportCSV(std::cout);


    std::cout << "\n=== Taxes (Question 6) ===\n";
budget.setPrintTaxRate(0.10);      // 10% tax on print items
budget.setElectronicTaxRate(0.05); // 5% tax on electronic items

Money basePrice = Money::of(1000);
Money printTax = budget.calculateTax(ResourceCategory::Book, basePrice);
Money printTotal = budget.costWithTax(ResourceCategory::Book, basePrice);

Money elecTax = budget.calculateTax(ResourceCategory::ElectronicResource, basePrice);
Money elecTotal = budget.costWithTax(ResourceCategory::ElectronicResource, basePrice);

std::cout << "Base Price: " << basePrice << "\n";
std::cout << "Print Item (10% tax): Tax = " << printTax << ", Total = " << printTotal << "\n";
std::cout << "Electronic Item (5% tax): Tax = " << elecTax << ", Total = " << elecTotal << "\n";


std::cout << "\n=== Title Limit Quota (Question 7) ===\n";
    Budget titleBudget(Money::of(10000));
    Quota titleQuota;
    titleQuota.maxTitles = 2; // Max 2 distinct titles allowed
    titleBudget.setQuota(ResourceCategory::Book, titleQuota);

    titleBudget.commit(ResourceCategory::Book, 2, Money::of(900), "B001");
    titleBudget.commit(ResourceCategory::Book, 1, Money::of(1200), "B002");

    std::string reason;
    auto status = titleBudget.evaluate(ResourceCategory::Book, 1, Money::of(450), "B003", reason);
    std::cout << "Attempting to buy 3rd distinct title (B003): " 
              << (status == Budget::Failure::None ? "APPROVED" : "REJECTED") << "\n";
    if (status != Budget::Failure::None) {
        std::cout << "  Reason: " << reason << "\n";
    }
    return 0;
}