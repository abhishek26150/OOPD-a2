// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#include "bookmgmt/AudioBook.h"

#include <iostream>

namespace bookmgmt {

AudioBook::AudioBook(std::string id, std::string title, std::vector<std::string> authors,
                     std::string isbn, std::string publisher, int year, Money unitPrice,
                     std::string accessUrl, int durationMinutes, std::string narrator,
                     std::string format)
    : ElectronicResource(std::move(id), std::move(title), std::move(publisher), year,
                         unitPrice, std::move(accessUrl), LicenseModel::Perpetual, Money::of(0)),
      authors_(std::move(authors)),
      isbn_(std::move(isbn)),
      durationMinutes_(durationMinutes),
      narrator_(std::move(narrator)),
      audioFormat_(std::move(format)) {}

void AudioBook::printDetails(std::ostream& os) const {
    ElectronicResource::printDetails(os);
    os << "  isbn: " << isbn_ << "\n";
    os << "  duration: " << durationMinutes_ << " mins\n";
    os << "  narrator: " << narrator_ << "\n";
    os << "  format: " << audioFormat_ << "\n";
}

}  // namespace bookmgmt