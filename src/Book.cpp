// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#include "bookmgmt/Book.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace bookmgmt {

Book::Book(std::string id, std::string title, std::vector<std::string> authors,
           std::string isbn, std::string publisher, int year, Money unitPrice,
           int edition, Binding binding)
    : Resource(std::move(id), std::move(title), std::move(publisher), year, unitPrice),
      authors_(std::move(authors)),
      isbn_(std::move(isbn)),
      edition_(edition),
      binding_(binding) {
    if (unitPrice.isNegative()) {
        throw std::invalid_argument("unit price cannot be negative");
    }
}

void Book::printDetails(std::ostream& os) const {
    os << "  isbn: " << isbn_ << "\n"
       << "  authors: " << joinAuthors(authors_) << "\n";
}

std::string joinAuthors(const std::vector<std::string>& authors) {
    if (authors.empty()) {
        return "";
    }
    if (authors.size() == 1) {
        return authors[0];
    }
    if (authors.size() == 2) {
        return authors[0] + " and " + authors[1];
    }

    std::ostringstream oss;
    for (size_t i = 0; i < authors.size() - 1; ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << authors[i];
    }
    oss << " and " << authors.back();
    return oss.str();
}

}  // namespace bookmgmt