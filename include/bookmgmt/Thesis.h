// Name: Abhishek Kumar Singh
// Roll Number: MT26150

#pragma once

#include <string>
#include <vector>

#include "bookmgmt/Resource.h"

namespace bookmgmt {

class Thesis : public Resource {
public:
    Thesis(std::string id, std::string title, std::string author,
           std::string advisor, std::string institution, std::string degree,
           int year, Money unitPrice = Money::of(0));

    const std::string& author() const { return author_; }
    const std::string& advisor() const { return advisor_; }
    const std::string& institution() const { return institution_; }
    const std::string& degree() const { return degree_; }

    ResourceCategory category() const override { return ResourceCategory::Thesis; }

protected:
    void printDetails(std::ostream& os) const override;

private:
    std::string author_;
    std::string advisor_;
    std::string institution_;
    std::string degree_;
};

}  // namespace bookmgmt