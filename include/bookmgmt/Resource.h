// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "bookmgmt/Money.h"

namespace bookmgmt {

// Question 12: Vendor offer structure inside bookmgmt namespace
struct VendorOffer {
    std::string vendorName;
    Money price;
};

enum class ResourceCategory { Book, ElectronicResource, Journal, EBook, AudioBook, Thesis };

const char* categoryName(ResourceCategory c);

class Resource {
public:
    Resource(std::string id, std::string title, std::string publisher,
             int year, Money unitPrice);
    virtual ~Resource() = default;

    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    const std::string& id() const { return id_; }
    const std::string& title() const { return title_; }
    const std::string& publisher() const { return publisher_; }
    int year() const { return year_; }
    Money unitPrice() const { return unitPrice_; }
    void setUnitPrice(Money price);

    void addVendorOffer(const std::string& vendorName, Money price) {
        vendorOffers_.push_back({vendorName, price});
    }

    const std::vector<VendorOffer>& vendorOffers() const { return vendorOffers_; }

    virtual ResourceCategory category() const = 0;
    virtual bool isDigital() const { return false; }

    virtual Money costFor(int quantity) const;

    void print(std::ostream& os) const;
    std::string summary() const;

protected:
    virtual void printDetails(std::ostream& os) const;
    static void requirePositive(int quantity);

private:
    std::string id_;
    std::string title_;
    std::string publisher_;
    int year_;
    Money unitPrice_;
    std::vector<VendorOffer> vendorOffers_;
};

std::ostream& operator<<(std::ostream& os, const Resource& r);

}  // namespace bookmgmt