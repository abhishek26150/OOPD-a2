// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#pragma once

#include <string>
#include <vector>

#include "bookmgmt/ElectronicResource.h"

namespace bookmgmt {

class AudioBook : public ElectronicResource {
public:
    AudioBook(std::string id, std::string title, std::vector<std::string> authors,
              std::string isbn, std::string publisher, int year, Money unitPrice,
              std::string accessUrl, int durationMinutes, std::string narrator,
              std::string format = "MP3");

    const std::vector<std::string>& authors() const { return authors_; }
    const std::string& isbn() const { return isbn_; }
    int durationMinutes() const { return durationMinutes_; }
    const std::string& narrator() const { return narrator_; }
    const std::string& audioFormat() const { return audioFormat_; }

    ResourceCategory category() const override { return ResourceCategory::AudioBook; }

protected:
    void printDetails(std::ostream& os) const override;

private:
    std::vector<std::string> authors_;
    std::string isbn_;
    int durationMinutes_;
    std::string narrator_;
    std::string audioFormat_;
};

}  // namespace bookmgmt