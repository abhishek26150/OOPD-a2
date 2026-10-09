// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#include "bookmgmt/EBook.h"
#include "bookmgmt/Book.h"
#include <ostream>

namespace bookmgmt {

const char* fileFormatName(FileFormat format) {
    switch (format) {
        case FileFormat::PDF:
            return "PDF";
        case FileFormat::EPUB:
            return "EPUB";
        case FileFormat::HTML:
            return "HTML";
    }
    return "Unknown";
}

EBook::EBook(std::string id, std::string title, std::vector<std::string> authors,
             std::string isbn, std::string publisher, int year, Money pricePerSeat,
             std::string accessUrl, FileFormat format, bool isDrmProtected,
             LicenseModel license, Money platformFee)
    : ElectronicResource(std::move(id), std::move(title), std::move(publisher), year,
                         pricePerSeat, std::move(accessUrl), license, platformFee),
      authors_(std::move(authors)),
      isbn_(std::move(isbn)),
      format_(format),
      isDrmProtected_(isDrmProtected) {}

void EBook::printDetails(std::ostream& os) const {
    // Print inherited ElectronicResource details first
    ElectronicResource::printDetails(os);
    os << "  isbn: " << isbn_ << "\n"
       << "  authors: " << joinAuthors(authors_) << "\n"
       << "  format: " << fileFormatName(format_) << "\n"
       << "  DRM protected: " << (isDrmProtected_ ? "yes" : "no") << "\n";
}

}  // namespace bookmgmt