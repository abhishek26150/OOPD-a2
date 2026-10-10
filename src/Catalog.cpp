#include "bookmgmt/Catalog.h"

#include <algorithm>
#include <cctype>
#include <ostream>
#include <sstream>
#include <stdexcept>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

namespace {
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return s;
}
}  // namespace

Resource& Catalog::add(std::unique_ptr<Resource> r) {
    if (!r) throw std::invalid_argument("cannot add a null resource");
    const std::string id = r->id();
    if (items_.count(id)) throw DuplicateIdError(id);
    Entry& e = items_[id];
    e.resource = std::move(r);
    return *e.resource;
}

bool Catalog::contains(const std::string& id) const { return items_.count(id) > 0; }

Resource* Catalog::find(const std::string& id) {
    auto it = items_.find(id);
    return it == items_.end() ? nullptr : it->second.resource.get();
}

const Resource* Catalog::find(const std::string& id) const {
    auto it = items_.find(id);
    return it == items_.end() ? nullptr : it->second.resource.get();
}

Resource& Catalog::get(const std::string& id) {
    if (Resource* r = find(id)) return *r;
    throw NotFoundError(id);
}

const Resource& Catalog::get(const std::string& id) const {
    if (const Resource* r = find(id)) return *r;
    throw NotFoundError(id);
}

void Catalog::remove(const std::string& id) {
    if (items_.erase(id) == 0) throw NotFoundError(id);
}

int Catalog::holdings(const std::string& id) const {
    auto it = items_.find(id);
    if (it == items_.end()) throw NotFoundError(id);
    return it->second.holdings;
}

void Catalog::addHoldings(const std::string& id, int units) {
    auto it = items_.find(id);
    if (it == items_.end()) throw NotFoundError(id);
    if (it->second.holdings + units < 0)
        throw std::invalid_argument("holdings cannot become negative");
    it->second.holdings += units;
}

std::vector<const Resource*> Catalog::where(
    const std::function<bool(const Resource&)>& pred) const {
    std::vector<const Resource*> out;
    for (const auto& [id, entry] : items_)
        if (pred(*entry.resource)) out.push_back(entry.resource.get());
    return out;
}

std::vector<const Resource*> Catalog::all() const {
    return where([](const Resource&) { return true; });
}

std::vector<const Resource*> Catalog::byCategory(ResourceCategory c) const {
    return where([c](const Resource& r) { return r.category() == c; });
}

std::vector<const Resource*> Catalog::searchTitle(const std::string& text) const {
    const std::string needle = lower(text);
    return where([&needle](const Resource& r) {
        return lower(r.title()).find(needle) != std::string::npos;
    });
}

std::vector<const Resource*> Catalog::byPriceRange(Money minPrice, Money maxPrice) const {
    return where([minPrice, maxPrice](const Resource& r) {
        return r.unitPrice() >= minPrice && r.unitPrice() <= maxPrice;
    });
}

std::vector<const Resource*> Catalog::findByAuthor(const std::string& author) const {
    std::vector<const Resource*> result;
    for (const auto& [id, entry] : items_) {
        const Resource* res = entry.resource.get();
        if (res) {
            std::ostringstream ss;
            res->print(ss);
            if (ss.str().find(author) != std::string::npos ||
                res->title().find(author) != std::string::npos) {
                result.push_back(res);
            }
        }
    }
    return result;
}

std::vector<const Resource*> Catalog::findByIsbnOrIssn(const std::string& identifier) const {
    std::vector<const Resource*> result;
    for (const auto& [id, entry] : items_) {
        const Resource* res = entry.resource.get();
        if (res) {
            std::ostringstream ss;
            res->print(ss);
            if (ss.str().find(identifier) != std::string::npos) {
                result.push_back(res);
            }
        }
    }
    return result;
}

std::vector<const Resource*> Catalog::findByYearRange(int startYear, int endYear) const {
    std::vector<const Resource*> result;
    for (const auto& [id, entry] : items_) {
        const Resource* res = entry.resource.get();
        if (res && res->year() >= startYear && res->year() <= endYear) {
            result.push_back(res);
        }
    }
    return result;
}

void Catalog::sortResults(std::vector<const Resource*>& results, SortField field, SortOrder order) {
    std::sort(results.begin(), results.end(), [field, order](const Resource* a, const Resource* b) {
        bool less = false;
        if (field == SortField::Title) {
            less = a->title() < b->title();
        } else if (field == SortField::Year) {
            less = a->year() < b->year();
        } else if (field == SortField::UnitPrice) {
            less = a->unitPrice() < b->unitPrice();
        }

        return order == SortOrder::Ascending ? less : !less;
    });
}

void Catalog::printDetailedReport(std::ostream& os) const {
    os << "=== Detailed Catalog Report ===\n";
    for (const auto& [id, entry] : items_) {
        os << "ID: " << id << " | Holdings: " << entry.holdings << "\n";
        if (entry.resource) {
            os << *entry.resource << "\n";
            os << "-----------------------------------\n";
        }
    }
}

void Catalog::exportCSV(std::ostream& os) const {
    os << "ID,Category,Title,Year,UnitPrice,Holdings\n";
    for (const auto& [id, entry] : items_) {
        if (entry.resource) {
            os << id << ","
               << categoryName(entry.resource->category()) << ",\""
               << entry.resource->title() << "\","
               << entry.resource->year() << ","
               << entry.resource->unitPrice() << ","
               << entry.holdings << "\n";
        }
    }
}

bool Catalog::borrowCopy(const std::string& id) {
    auto it = items_.find(id);
    if (it == items_.end()) throw NotFoundError(id);

    if (it->second.borrowedCopies < it->second.holdings) {
        it->second.borrowedCopies++;
        return true;
    }
    return false;
}

bool Catalog::returnCopy(const std::string& id) {
    auto it = items_.find(id);
    if (it == items_.end()) throw NotFoundError(id);

    if (it->second.borrowedCopies > 0) {
        it->second.borrowedCopies--;
        return true;
    }
    return false;
}

int Catalog::activeBorrows(const std::string& id) const {
    auto it = items_.find(id);
    if (it == items_.end()) throw NotFoundError(id);
    return it->second.borrowedCopies;
}

bool Catalog::openSession(const std::string& id) {
    auto it = items_.find(id);
    if (it == items_.end()) throw NotFoundError(id);

    if (it->second.activeSessions < it->second.holdings) {
        it->second.activeSessions++;
        return true;
    }
    return false;
}

bool Catalog::closeSession(const std::string& id) {
    auto it = items_.find(id);
    if (it == items_.end()) throw NotFoundError(id);

    if (it->second.activeSessions > 0) {
        it->second.activeSessions--;
        return true;
    }
    return false;
}

int Catalog::activeSessions(const std::string& id) const {
    auto it = items_.find(id);
    if (it == items_.end()) throw NotFoundError(id);
    return it->second.activeSessions;
}

}  // namespace bookmgmt