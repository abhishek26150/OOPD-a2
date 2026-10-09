// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#pragma once

#include <string>
#include <vector>
#include "bookmgmt/ElectronicResource.h"

namespace bookmgmt {

enum class FileFormat { PDF, EPUB, HTML };

const char* fileFormatName(FileFormat format);

// Design Answer (Q2):
// Code Duplication between Book and EBook:
// Both Book and EBook duplicate storing and processing metadata like `authors` (std::vector<std::string>)
// and `isbn` (std::string), as well as helper routines like `joinAuthors()`.
//
// How to avoid duplication:
// 1. Use Multiple Inheritance or Mixin Interfaces: Define a `BookMetadata` interface or class containing `authors` and `isbn`.
// 2. Composition / Strategy Pattern: Create a helper struct `BookDetails` holding `authors` and `isbn`, included as a member in both Book and EBook.

class EBook : public ElectronicResource {
public:
    EBook(std::string id, std::string title, std::vector<std::string> authors,
          std::string isbn, std::string publisher, int year, Money pricePerSeat,
          std::string accessUrl, FileFormat format = FileFormat::PDF,
          bool isDrmProtected = false,
          LicenseModel license = LicenseModel::AnnualSubscription,
          Money platformFee = Money{});

    const std::vector<std::string>& authors() const { return authors_; }
    const std::string& isbn() const { return isbn_; }
    FileFormat format() const { return format_; }
    bool isDrmProtected() const { return isDrmProtected_; }

    ResourceCategory category() const override { return ResourceCategory::EBook; }

protected:
    void printDetails(std::ostream& os) const override;

private:
    std::vector<std::string> authors_;
    std::string isbn_;
    FileFormat format_;
    bool isDrmProtected_;
};

}  // namespace bookmgmt