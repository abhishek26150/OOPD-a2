// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#include "bookmgmt/Journal.h"
#include <ostream>
#include <stdexcept>

namespace bookmgmt {

Journal::Journal(std::string id,
                 std::string title,
                 std::string publisher,
                 int year,
                 Money unitPrice,
                 std::string issn,
                 int issuesPerYear,
                 int subscriptionYears)
    : Resource(std::move(id), std::move(title), std::move(publisher), year, unitPrice),
      issn_(std::move(issn)),
      issuesPerYear_(issuesPerYear),
      subscriptionYears_(subscriptionYears) {
    if (issn_.empty()) {
        throw std::invalid_argument("issn must not be empty");
    }
    if (issuesPerYear <= 0) {
        throw std::invalid_argument("issues per year must be positive");
    }
    if (subscriptionYears <= 0) {
        throw std::invalid_argument("subscription years must be positive");
    }
}

Money Journal::costFor(int quantity) const {
    requirePositive(quantity);
    return unitPrice() * quantity * subscriptionYears_;
}

void Journal::printDetails(std::ostream& os) const {
    os << "  issn: " << issn_ << "\n"
       << "  issues per year: " << issuesPerYear_ << "\n"
       << "  subscription years: " << subscriptionYears_ << "\n";
}

}  // namespace bookmgmt