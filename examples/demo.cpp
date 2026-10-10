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


    std::cout << "\n=== Order Cancellation (Question 8) ===\n";
    Catalog cancelCat;
    cancelCat.emplace<Book>("B999", "Refactoring", std::vector<std::string>{"Fowler"}, "999", "Addison", 2018, Money::of(500));
    Budget cancelBudget(Money::of(5000));
    AcquisitionManager cancelAcq(cancelCat, cancelBudget);

    auto ord = cancelAcq.purchase("B999", 2);
    std::cout << "Purchased Order #" << ord.orderNo << " (Spent: " << cancelBudget.spent() << ", Holdings: " << cancelCat.holdings("B999") << ")\n";

    cancelAcq.cancelOrder(ord.orderNo);
    std::cout << "After Cancelling Order #" << ord.orderNo << ": Spent: " << cancelBudget.spent() << ", Holdings: " << cancelCat.holdings("B999") << "\n";

    std::cout << "\n=== Department Budgets (Question 9) ===\n";
    Catalog deptCat;
    deptCat.emplace<Book>("B001", "Clean Code", std::vector<std::string>{"Martin"}, "123", "Prentice", 2008, Money::of(450));
    
    Budget mainB(Money::of(20000));
    AcquisitionManager deptAcq(deptCat, mainB);

    auto csB = std::make_shared<Budget>(Money::of(2000));
    csB->setQuota(ResourceCategory::Book, Quota{3, Money::of(1500)});
    deptAcq.addDepartmentBudget("Computer Science", csB);

    auto ordDept = deptAcq.purchase("B001", 2, "Computer Science");
    std::cout << "Charged " << ordDept.quantity << " copies of " << ordDept.resourceId 
              << " to [" << ordDept.department << "]. Dept Spent: " << csB->spent() << "\n";


    std::cout << "\n=== Year-End Budget Rollover (Question 10) ===\n";
    Budget budget2025(Money::of(10000));
    budget2025.commit(ResourceCategory::Book, 2, Money::of(4000)); // Spent 4000, unspent 6000

    std::cout << "2025 Budget: Total " << budget2025.total() << ", Spent " << budget2025.spent() 
              << ", Remaining " << budget2025.remaining() << "\n";

    // Carry forward 50% of remaining 6000 = +3000
    Budget budget2026 = Budget::createRollover(budget2025, 50.0);
    std::cout << "2026 Budget (after 50% rollover): Total " << budget2026.total() 
              << ", Spent " << budget2026.spent() << ", Remaining " << budget2026.remaining() << "\n";          


std::cout << "\n=== All-or-Nothing Batch Processing (Question 11) ===\n";
    Catalog batchCat;
    batchCat.emplace<Book>("B1", "Book 1", std::vector<std::string>{"A"}, "111", "Pub", 2020, Money::of(300));
    batchCat.emplace<Book>("B2", "Book 2", std::vector<std::string>{"B"}, "222", "Pub", 2021, Money::of(800));
    Budget batchBudget(Money::of(1000));
    AcquisitionManager batchAcq(batchCat, batchBudget);

    std::vector<PurchaseRequest> reqs = { {"B1", 1}, {"B2", 1} }; // 300 + 800 = 1100 > 1000
    auto res = batchAcq.processBatch(reqs, true);

    std::cout << "Batch processed with All-or-Nothing=true. Requests count: " << res.size() << "\n";
    std::cout << "Approved: " << (res[0].approved ? "Yes" : "No") << ", Budget spent: " << batchBudget.spent() << "\n";              




    std::cout << "\n=== Vendor Selection (Question 12) ===\n";
    Catalog vendorCat;
    vendorCat.emplace<Book>("B001", "Clean Code", std::vector<std::string>{"Martin"}, "123", "Prentice", 2008, Money::of(500));
    
    Resource* vRes = vendorCat.find("B001");
    vRes->addVendorOffer("Amazon", Money::of(480));
    vRes->addVendorOffer("BookDepository", Money::of(420));
    vRes->addVendorOffer("LocalStore", Money::of(450));

    Budget vendorBudget(Money::of(5000));
    AcquisitionManager vendorAcq(vendorCat, vendorBudget);

    auto vOrder = vendorAcq.purchase("B001", 1);
    std::cout << "Purchased B001 from cheapest vendor: " << vOrder.vendor 
              << " at Price: " << vOrder.cost << "\n";

    std::cout << "\n=== Extended Searches (Question 13) ===\n";
    Catalog searchCat;
    searchCat.emplace<Book>("B01", "Clean Code", std::vector<std::string>{"Robert Martin"}, "978-0132350884", "Prentice", 2008, Money::of(400));
    searchCat.emplace<Book>("B02", "Design Patterns", std::vector<std::string>{"Erich Gamma"}, "978-0201633610", "Addison", 1994, Money::of(500));

    auto found = searchCat.findByYearRange(2000, 2010);
    std::cout << "Found " << found.size() << " book(s) published between 2000-2010: " 
              << (found.empty() ? "" : found[0]->title()) << "\n";
      
              
    std::cout << "\n=== Lending System (Question 14) ===\n";
    Catalog lendCat;
    lendCat.emplace<Book>("B01", "Clean Code", std::vector<std::string>{"Martin"}, "123", "Prentice", 2008, Money::of(400));
    lendCat.addHoldings("B01", 1);

    lendCat.emplace<ElectronicResource>("R01", "ACM Digital Library", "ACM", 2026, Money::of(100), "https://acm.example");
    lendCat.addHoldings("R01", 1);

    std::cout << "Borrowing print copy B01: " << (lendCat.borrowCopy("B01") ? "Success" : "Failed") << "\n";
    std::cout << "Borrowing second copy (Exceeds holdings): " << (lendCat.borrowCopy("B01") ? "Success" : "Failed (Limit Reached)") << "\n";
    std::cout << "Opening e-resource session R01: " << (lendCat.openSession("R01") ? "Success" : "Failed") << "\n";
    std::cout << "Opening second session (Exceeds seats): " << (lendCat.openSession("R01") ? "Success" : "Failed (Seats Full)") << "\n";
    
    

    
    return 0;
}