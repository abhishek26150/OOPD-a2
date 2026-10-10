// Name: Abhishek Kumar Singh
// Roll Number: MT26150

// Minimal self-contained test runner (no external framework needed).

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "bookmgmt/bookmgmt.h"

#include "bookmgmt/EBook.h"
#include "bookmgmt/AudioBook.h"
#include "bookmgmt/Thesis.h"

using namespace bookmgmt;

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond)                                                               \
    do {                                                                          \
        ++g_checks;                                                               \
        if (!(cond)) {                                                            \
            ++g_failures;                                                         \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond  \
                      << "\n";                                                    \
        }                                                                         \
    } while (0)

#define CHECK_THROWS(expr, ExType)             \
    do {                                       \
        bool thrown_ = false;                  \
        try {                                  \
            (void)(expr);                      \
        } catch (const ExType&) {              \
            thrown_ = true;                    \
        } catch (...) {                        \
        }                                      \
        CHECK(thrown_ && "expected " #ExType); \
    } while (0)

static void testMoney() {
    CHECK(Money::of(12, 5).toString() == "12.05");
    CHECK(Money::of(-3, 50).toString() == "-3.50");
    CHECK(Money::fromMinor(7).toString() == "0.07");
    CHECK(Money::of(10) + Money::of(0, 50) == Money::fromMinor(1050));
    CHECK(Money::of(3) * 4 == Money::of(12));
    CHECK(Money::of(1) < Money::of(2));
    CHECK_THROWS(Money::of(1, 100), std::invalid_argument);
}

static void testResourcesAndCost() {
    Book b("B1", "T", {"A", "B", "C"}, "isbn", "P", 2020, Money::of(100));
    CHECK(b.category() == ResourceCategory::Book);
    CHECK(!b.isDigital());
    CHECK(b.costFor(3) == Money::of(300));
    CHECK_THROWS(b.costFor(0), std::invalid_argument);
    CHECK(joinAuthors(b.authors()) == "A, B and C");

    ElectronicResource e("R1", "DB", "P", 2026, Money::of(10), "url",
                         LicenseModel::AnnualSubscription, Money::of(100));
    CHECK(e.isDigital());
    CHECK(e.costFor(5) == Money::of(150));

    CHECK(e.category() == ResourceCategory::ElectronicResource);

    // Polymorphism through a base-class reference
    const Resource& r = e;
    CHECK(r.costFor(1) == Money::of(110));
    std::ostringstream os;
    os << r;
    CHECK(os.str().find("platform fee: 100.00") != std::string::npos);

    CHECK_THROWS(Book("", "T", {}, "", "", 2000, Money::of(1)), std::invalid_argument);
    CHECK_THROWS(Book("B", "T", {}, "", "", 2000, Money::fromMinor(-1)),
                 std::invalid_argument);
}

static void testCatalog() {
    Catalog c;
    c.emplace<Book>("B1", "Clean Code", std::vector<std::string>{"M"}, "i", "P", 2008,
                    Money::of(1));
    c.emplace<Book>("B2", "Clean Architecture", std::vector<std::string>{"M"}, "i", "P",
                    2017, Money::of(1));
    c.emplace<ElectronicResource>("R1", "ACM Digital Library", "ACM", 2026, Money::of(1),
                                  "url");

    CHECK(c.size() == 3);
    CHECK(c.contains("B1"));
    CHECK(c.find("nope") == nullptr);
    CHECK_THROWS(c.get("nope"), NotFoundError);
    CHECK_THROWS(c.emplace<Book>("B1", "dup", std::vector<std::string>{}, "", "", 1,
                                 Money::of(1)),
                 DuplicateIdError);

    CHECK(c.searchTitle("clean").size() == 2);
    CHECK(c.byCategory(ResourceCategory::ElectronicResource).size() == 1);
    CHECK(c.where([](const Resource& r) { return r.isDigital(); }).size() == 1);

    CHECK(c.holdings("B1") == 0);
    c.addHoldings("B1", 3);
    CHECK(c.holdings("B1") == 3);
    CHECK_THROWS(c.addHoldings("B1", -5), std::invalid_argument);

    c.remove("R1");
    CHECK(c.size() == 2);
    CHECK_THROWS(c.remove("R1"), NotFoundError);
}

static void testBudget() {
    Budget b(Money::of(1000));
    b.setQuota(ResourceCategory::Book, {5, Money::of(400)});

    CHECK(b.check(ResourceCategory::Book, 2, Money::of(200)).empty());
    CHECK(!b.check(ResourceCategory::Book, 6, Money::of(10)).empty());   // units
    CHECK(!b.check(ResourceCategory::Book, 1, Money::of(401)).empty());  // spend
    CHECK(!b.check(ResourceCategory::ElectronicResource, 1, Money::of(1001)).empty());  // overall
    CHECK(b.check(ResourceCategory::ElectronicResource, 1, Money::of(900)).empty());    // no quota

    b.commit(ResourceCategory::Book, 4, Money::of(300));
    CHECK(b.spent() == Money::of(300));
    CHECK(*b.unitsRemaining(ResourceCategory::Book) == 1);
    CHECK(*b.spendRemaining(ResourceCategory::Book) == Money::of(100));
    CHECK(!b.unitsRemaining(ResourceCategory::ElectronicResource).has_value());

    CHECK_THROWS(b.commit(ResourceCategory::Book, 2, Money::of(10)), QuotaExceededError);
    CHECK_THROWS(b.commit(ResourceCategory::ElectronicResource, 1, Money::of(800)),
                 BudgetExceededError);
    CHECK_THROWS(b.commit(ResourceCategory::ElectronicResource, 0, Money::of(1)),
                 std::invalid_argument);
    CHECK(b.spent() == Money::of(300));  // failed commits changed nothing
}

static void testAcquisition() {
    Catalog c;
    c.emplace<Book>("B1", "Book", std::vector<std::string>{"A"}, "i", "P", 2020,
                    Money::of(100));
    c.emplace<ElectronicResource>("R1", "DB", "P", 2026, Money::of(10), "url",
                                  LicenseModel::AnnualSubscription, Money::of(50));
    Budget b(Money::of(500));
    b.setQuota(ResourceCategory::Book, {3, Money::of(1000)});
    AcquisitionManager acq(c, b);

    CHECK(acq.quote("R1", 5) == Money::of(100));
    std::string why;
    CHECK(acq.canPurchase("B1", 3, &why) && why.empty());
    CHECK(!acq.canPurchase("B1", 4, &why) && !why.empty());
    CHECK(!acq.canPurchase("nope", 1, &why));

    const auto& rec = acq.purchase("B1", 2);
    CHECK(rec.approved && rec.cost == Money::of(200) && rec.orderNo == 1);
    CHECK(c.holdings("B1") == 2);

    CHECK_THROWS(acq.purchase("B1", 2), QuotaExceededError);
    CHECK_THROWS(acq.purchase("nope", 1), NotFoundError);
    CHECK(acq.history().size() == 1);  // exceptions don't record

    auto res = acq.processBatch({{"R1", 10}, {"R1", 100}, {"B1", 1}, {"zzz", 1}, {"B1", 0}});
    CHECK(res.size() == 5);
    CHECK(res[0].approved && res[0].cost == Money::of(150));
    CHECK(!res[1].approved);  // 1050 > remaining 150
    CHECK(res[2].approved);
    CHECK(!res[3].approved && res[3].reason.find("not found") != std::string::npos);
    CHECK(!res[4].approved);
    CHECK(acq.totalSpent() == Money::of(450));
    CHECK(b.spent() == acq.totalSpent());
    CHECK(c.holdings("R1") == 10 && c.holdings("B1") == 3);
    CHECK(acq.history().size() == 6);
}

// Appended Q1: Journal Test
static void testJournal() {
    Journal j("J1", "IEEE Software", "IEEE", 2023, Money::of(120), "0740-7459", 6, 2);
    CHECK(j.category() == ResourceCategory::Journal);
    CHECK(!j.isDigital());
    CHECK(j.costFor(3) == Money::of(120 * 3 * 2));
    CHECK(j.issn() == "0740-7459");
    CHECK(j.issuesPerYear() == 6);
    CHECK(j.subscriptionYears() == 2);
    CHECK_THROWS(Journal("J2", "Bad", "Pub", 2023, Money::of(100), "issn", 4, 0), std::invalid_argument);
    CHECK_THROWS(j.costFor(0), std::invalid_argument);
}
void test_question_2_ebook() {
    using namespace bookmgmt;

    EBook ebook("EB001", "C++ Primer", {"Stanley Lippman", "Josée Lajoie"},
                "978-0321714114", "Addison-Wesley", 2012, Money::of(250),
                "https://ebooks.example/cpp", FileFormat::EPUB, true);

    CHECK(ebook.id() == "EB001");
    CHECK(ebook.title() == "C++ Primer");
    CHECK(ebook.category() == ResourceCategory::EBook);
    CHECK(ebook.isDigital() == true);
    CHECK(ebook.format() == FileFormat::EPUB);
    CHECK(ebook.isDrmProtected() == true);
    CHECK(ebook.costFor(4) == Money::of(1000));

    // Test quota & catalog integration
    Catalog catalog;
    catalog.emplace<EBook>("EB001", "C++ Primer", std::vector<std::string>{"Lippman"},
                           "978-0321714114", "AW", 2012, Money::of(200),
                           "https://ebooks.example/cpp");

    Budget budget(Money::of(5000));
    Quota q;
    q.maxUnits = 5;
    q.maxSpend = Money::of(1000);
    budget.setQuota(ResourceCategory::EBook, q);

    AcquisitionManager acq(catalog, budget);
    CHECK(acq.canPurchase("EB001", 3) == true);
    auto order = acq.purchase("EB001", 3);
    CHECK(order.approved == true);
}

void test_question_3_audiobook_and_thesis() {
    using namespace bookmgmt;

    // Test AudioBook
    AudioBook abook("AB001", "The Pragmatic Programmer", {"Andrew Hunt", "David Thomas"},
                    "978-0135957059", "Addison-Wesley", 2019, Money::of(350),
                    "https://audio.example/pp", 600, "Ray Chase", "MP3");

    CHECK(abook.id() == "AB001");
    CHECK(abook.title() == "The Pragmatic Programmer");
    CHECK(abook.category() == ResourceCategory::AudioBook);
    CHECK(abook.isDigital() == true);
    CHECK(abook.durationMinutes() == 600);
    CHECK(abook.narrator() == "Ray Chase");
    CHECK(abook.audioFormat() == "MP3");

    // Test Thesis
    Thesis thesis("TH001", "Deep Learning Systems Design", "Alex Rivera",
                  "Dr. Sarah Connor", "IIIT Delhi", "Ph.D.", 2024, Money::of(0));

    CHECK(thesis.id() == "TH001");
    CHECK(thesis.category() == ResourceCategory::Thesis);
    CHECK(thesis.author() == "Alex Rivera");
    CHECK(thesis.advisor() == "Dr. Sarah Connor");
    CHECK(thesis.institution() == "IIIT Delhi");
    CHECK(thesis.degree() == "Ph.D.");
    CHECK(thesis.costFor(1) == Money::of(0));
}


void test_question_4_catalog_queries() {
    using namespace bookmgmt;

    Catalog cat;
    cat.emplace<Book>("B01", "Clean Code", std::vector<std::string>{"Robert Martin"}, "123", "Prentice", 2008, Money::of(450), 400);
    cat.emplace<Book>("B02", "Code Complete", std::vector<std::string>{"Steve McConnell"}, "456", "Microsoft", 2004, Money::of(600), 900);
    cat.emplace<Thesis>("TH01", "Systems Design", "Alice", "Dr. Bob", "IIIT", "Ph.D.", 2023, Money::of(0));

    // Category Query
    auto books = cat.byCategory(ResourceCategory::Book);
    CHECK(books.size() == 2);

    // Title Search
    auto codeResults = cat.searchTitle("code");
    CHECK(codeResults.size() == 2);

    // Price Range Query
    auto cheap = cat.byPriceRange(Money::of(0), Money::of(500));
    CHECK(cheap.size() == 2);

    // Predicate Query
    auto recent = cat.where([](const Resource& r) { return r.year() >= 2008; });
    CHECK(recent.size() == 2);
}

void test_question_5_reports_and_export() {
    using namespace bookmgmt;

    Catalog cat;
    cat.emplace<Book>("B01", "Clean Code", std::vector<std::string>{"Robert Martin"}, "123", "Prentice", 2008, Money::of(450), 400);
    cat.addHoldings("B01", 5);

    std::stringstream ss;
    cat.exportCSV(ss);
    std::string csvOutput = ss.str();

    CHECK(csvOutput.find("ID,Category,Title,Year,UnitPrice,Holdings") != std::string::npos);
    CHECK(csvOutput.find("B01,Book,\"Clean Code\",2008,450.00,5") != std::string::npos);
}

void test_question_6_taxes() {
    using namespace bookmgmt;

    Budget budget(Money::of(10000));
    budget.setPrintTaxRate(0.10);      // 10% tax on print
    budget.setElectronicTaxRate(0.05); // 5% tax on electronic

    Money base = Money::of(1000);
    Money printPostTax = budget.costWithTax(ResourceCategory::Book, base);
    Money elecPostTax = budget.costWithTax(ResourceCategory::ElectronicResource, base);

    CHECK(printPostTax == Money::of(1100));
    CHECK(elecPostTax == Money::of(1050));
}

void test_question_7_title_limit_quota() {
    using namespace bookmgmt;

    Budget budget(Money::of(10000));
    Quota q;
    q.maxUnits = 10;
    q.maxSpend = Money::of(5000);
    q.maxTitles = 2; // Only 2 distinct titles allowed for Book

    budget.setQuota(ResourceCategory::Book, q);

    // First title: B01 -> Allowed
    budget.commit(ResourceCategory::Book, 2, Money::of(500), "B01");
    CHECK(budget.usageFor(ResourceCategory::Book).distinctTitles() == 1);

    // Same title B01 again -> Allowed (doesn't increase distinct title count)
    budget.commit(ResourceCategory::Book, 1, Money::of(250), "B01");
    CHECK(budget.usageFor(ResourceCategory::Book).distinctTitles() == 1);

    // Second distinct title: B02 -> Allowed
    budget.commit(ResourceCategory::Book, 1, Money::of(300), "B02");
    CHECK(budget.usageFor(ResourceCategory::Book).distinctTitles() == 2);

    // Third distinct title: B03 -> Exceeds title quota!
    std::string why;
    auto failure = budget.evaluate(ResourceCategory::Book, 1, Money::of(100), "B03", why);
    CHECK(failure == Budget::Failure::Quota);
    CHECK(why.find("title quota exceeded") != std::string::npos);
}


void test_question_8_cancellation() {
    using namespace bookmgmt;

    Catalog cat;
    cat.emplace<Book>("B01", "Clean Code", std::vector<std::string>{"Martin"}, "123", "Prentice", 2008, Money::of(400));
    Budget budget(Money::of(2000));
    budget.setQuota(ResourceCategory::Book, Quota{5, Money::of(2000)});
    AcquisitionManager acq(cat, budget);

    // 1. Purchase order #1
    auto order = acq.purchase("B01", 3);
    CHECK(order.orderNo == 1);
    CHECK(cat.holdings("B01") == 3);
    CHECK(budget.spent() == Money::of(1200));

    // 2. Cancel order #1 (Generates Order #2)
    auto cancelRec = acq.cancelOrder(1);
    CHECK(cancelRec.orderNo == 2);
    CHECK(cancelRec.approved == true);
    CHECK(cancelRec.isCancellation == true);
    
    // 3. Verify holdings and budget refund
    CHECK(cat.holdings("B01") == 0);
    CHECK(budget.spent() == Money::of(0));
    CHECK(acq.history().size() == 2);
}
void test_question_9_department_budgets() {
    using namespace bookmgmt;

    Catalog cat;
    cat.emplace<Book>("B01", "Clean Code", std::vector<std::string>{"Martin"}, "123", "Prentice", 2008, Money::of(100));
    
    Budget mainBudget(Money::of(10000));
    AcquisitionManager acq(cat, mainBudget);

    auto csBudget = std::make_shared<Budget>(Money::of(10000));
    csBudget->setQuota(ResourceCategory::Book, Quota{2, Money::of(5000)});
    
    acq.addDepartmentBudget("CS", csBudget);

    // Buy 1 copy for CS department -> Approved
    auto order1 = acq.purchase("B01", 1, "CS");
    CHECK(order1.approved == true);
    CHECK(order1.department == "CS");
    CHECK(csBudget->spent() == Money::of(100));

    // Exceed CS unit quota (buying 2 more = 3 total, max 2 allowed)
    std::string reason;
    bool can = acq.canPurchase("B01", 2, "CS", &reason);
    CHECK(can == false);
    CHECK(reason.find("unit quota exceeded") != std::string::npos);
}

void test_question_10_year_end_rollover() {
    using namespace bookmgmt;

    Budget budget2025(Money::of(10000));
    budget2025.setQuota(ResourceCategory::Book, Quota{5, Money::of(8000)});
    
    // Spend 6000, remaining = 4000
    budget2025.commit(ResourceCategory::Book, 2, Money::of(6000));
    CHECK(budget2025.remaining() == Money::of(4000));

    // Carry forward 50% of unspent amount (50% of 4000 = 2000)
    Budget budget2026 = Budget::createRollover(budget2025, 50.0);

    CHECK(budget2026.total() == Money::of(12000)); // 10000 + 2000
    CHECK(budget2026.spent() == Money::of(0));     // Spent reset for new year
    CHECK(budget2026.remaining() == Money::of(12000));
    
    // Verify quota carries over
    auto q = budget2026.quotaFor(ResourceCategory::Book);
    CHECK(q.has_value());
    CHECK(q->maxUnits == 5);
}


void test_question_11_all_or_nothing_batch() {
    using namespace bookmgmt;

    Catalog cat;
    cat.emplace<Book>("B01", "Clean Code", std::vector<std::string>{"Martin"}, "123", "Prentice", 2008, Money::of(400));
    cat.emplace<Book>("B02", "Design Patterns", std::vector<std::string>{"GoF"}, "456", "Addison", 1994, Money::of(700));
    
    Budget budget(Money::of(1000)); // Total 1000 budget
    AcquisitionManager acq(cat, budget);

    std::vector<PurchaseRequest> batch = {
        {"B01", 1}, // Cost 400 (Valid)
        {"B02", 1}  // Cost 700 (400 + 700 = 1100 > 1000 Budget)
    };

    // processBatch with allOrNothing = true
    auto results = acq.processBatch(batch, true);

    CHECK(results.size() == 2);
    CHECK(results[0].approved == false);
    CHECK(results[1].approved == false);
    CHECK(budget.spent() == Money::of(0)); // Nothing bought
    CHECK(cat.holdings("B01") == 0);       // Holdings unchanged
}


int main() {
    testMoney();
    testResourcesAndCost();
    testCatalog();
    testBudget();
    testAcquisition();
    testJournal();

    test_question_2_ebook();
    test_question_3_audiobook_and_thesis();
    test_question_4_catalog_queries();
    test_question_5_reports_and_export();
    test_question_6_taxes();
    test_question_7_title_limit_quota();
    test_question_8_cancellation();
    test_question_9_department_budgets();
    test_question_10_year_end_rollover();
    test_question_11_all_or_nothing_batch();


    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}