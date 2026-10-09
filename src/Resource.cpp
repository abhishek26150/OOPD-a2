// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#include "bookmgmt/Resource.h"
#include <iostream>
#include <stdexcept>

namespace bookmgmt {

const char* categoryName(ResourceCategory c) {
    switch (c) {
        case ResourceCategory::Book:
            return "Book";
        case ResourceCategory::ElectronicResource:
            return "ElectronicResource";
        case ResourceCategory::Journal:
            return "Journal";
    }
    return "Unknown";
}

Resource::Resource(std::string id, std::string title, std::string publisher,
                   int year, Money unitPrice)
    : id_(std::move(id)),
      title_(std::move(title)),
      publisher_(std::move(publisher)),
      year_(year),
      unitPrice_(unitPrice) {
    if (id_.empty()) {
        throw std::invalid_argument("resource id must not be empty");
    }
    if (unitPrice_.isNegative()) {
        throw std::invalid_argument("unit price cannot be negative");
    }
}

void Resource::setUnitPrice(Money price) {
    if (price.isNegative()) {
        throw std::invalid_argument("unit price cannot be negative");
    }
    unitPrice_ = price;
}

Money Resource::costFor(int quantity) const {
    requirePositive(quantity);
    return unitPrice_ * quantity;
}

void Resource::print(std::ostream& os) const {
    os << categoryName(category()) << " " << id_ << "\n"
       << "  title: " << title_ << "\n"
       << "  publisher: " << publisher_ << "\n"
       << "  year: " << year_ << "\n"
       << "  unit price: " << unitPrice_ << "\n";
    printDetails(os);
}

std::string Resource::summary() const {
    return "[" + std::string(categoryName(category())) + "] " + id_ + "  " + title_ +
           " (" + std::to_string(year_) + ")  @ " + unitPrice_.toString();
}

void Resource::printDetails(std::ostream& /*os*/) const {
    // Default implementation does nothing
}

void Resource::requirePositive(int quantity) {
    if (quantity <= 0) {
        throw std::invalid_argument("quantity must be positive");
    }
}

std::ostream& operator<<(std::ostream& os, const Resource& r) {
    r.print(os);
    return os;
}

}  // namespace bookmgmt