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






int main() {
    testMoney();
    testResourcesAndCost();
    testCatalog();
    testBudget();
    testAcquisition();
    testJournal();

    test_question_2_ebook();
    test_question_3_audiobook_and_thesis();



    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}