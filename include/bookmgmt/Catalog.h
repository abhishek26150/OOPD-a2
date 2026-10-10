#pragma once
// Catalog: owns every Resource, keyed by its id, and tracks copies/seats held.

#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "bookmgmt/Resource.h"

namespace bookmgmt {

enum class SortField { Title, Year, UnitPrice };
enum class SortOrder { Ascending, Descending };

class Catalog {
public:
    // Takes ownership. Throws DuplicateIdError if the id is already present.
    Resource& add(std::unique_ptr<Resource> r);

    // Convenience: catalog.emplace<Book>(ctor args...)
    template <typename T, typename... Args>
    T& emplace(Args&&... args) {
        auto p = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *p;
        add(std::move(p));
        return ref;
    }

    bool contains(const std::string& id) const;
    Resource* find(const std::string& id);              // nullptr if absent
    const Resource* find(const std::string& id) const;  // nullptr if absent
    Resource& get(const std::string& id);               // throws NotFoundError
    const Resource& get(const std::string& id) const;   // throws NotFoundError
    void remove(const std::string& id);                 // throws NotFoundError

    std::size_t size() const { return items_.size(); }
    bool empty() const { return items_.empty(); }

    // Holdings: number of copies (print) or seats (electronic) owned.
    int holdings(const std::string& id) const;
    void addHoldings(const std::string& id, int units);

    // Queries. Results are in id order and point into the catalog.
    std::vector<const Resource*> all() const;
    std::vector<const Resource*> byCategory(ResourceCategory c) const;
    std::vector<const Resource*> searchTitle(const std::string& text) const;  // case-insensitive
    std::vector<const Resource*> byPriceRange(Money minPrice, Money maxPrice) const;
    std::vector<const Resource*> where(
        const std::function<bool(const Resource&)>& pred) const;

    // Question 7: Sorting extension
    static void sortResults(std::vector<const Resource*>& results, 
                            SortField field, 
                            SortOrder order = SortOrder::Ascending);

    // Reporting & Export
    void printDetailedReport(std::ostream& os) const;
    void exportCSV(std::ostream& os) const;

        // Question 13

    std::vector<const Resource*> findByAuthor(const std::string& author) const;
    std::vector<const Resource*> findByIsbnOrIssn(const std::string& identifier) const;
    std::vector<const Resource*> findByYearRange(int startYear, int endYear) const;

private:
    struct Entry {
        std::unique_ptr<Resource> resource;
        int holdings = 0;
    };
    std::map<std::string, Entry> items_;
};

}  // namespace bookmgmt