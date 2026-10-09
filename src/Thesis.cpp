// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#include "bookmgmt/Thesis.h"

#include <iostream>

namespace bookmgmt {

Thesis::Thesis(std::string id, std::string title, std::string author,
               std::string advisor, std::string institution, std::string degree,
               int year, Money unitPrice)
    : Resource(std::move(id), std::move(title), std::move(institution), year, unitPrice),
      author_(std::move(author)),
      advisor_(std::move(advisor)),
      institution_(publisher()),
      degree_(std::move(degree)) {}

void Thesis::printDetails(std::ostream& os) const {
    os << "Thesis " << id() << "\n";
    os << "  title: " << title() << "\n";
    os << "  author: " << author_ << "\n";
    os << "  advisor: " << advisor_ << "\n";
    os << "  degree: " << degree_ << "\n";
    os << "  institution: " << institution_ << "\n";
    os << "  year: " << year() << "\n";
}

}  // namespace bookmgmt