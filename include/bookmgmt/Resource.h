// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#pragma once
// Resource: abstract base class for every item the library can hold or buy.

#include <iosfwd>
#include <string>

#include "bookmgmt/Money.h"

namespace bookmgmt {

enum class ResourceCategory { Book, ElectronicResource, Journal };

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
};

std::ostream& operator<<(std::ostream& os, const Resource& r);

}  // namespace bookmgmt