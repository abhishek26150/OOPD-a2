# Assignment 2 - ANSWERS.md

**Name:** Abhishek Kumar Singh  
**Roll Number:** MT26150  

---

## 1. Questions Attempted
All questions from **Q1 through Q16** have been successfully implemented, tested, and verified.

---

## 2. Design Decisions & Implementation Highlights

- **Part A: Class Hierarchy (Q1–Q3)**
  - `Journal` extends `Resource` directly; tracks ISSN, issues per year, and subscription duration.
  - `EBook` inherits pricing and licensing logic from `ElectronicResource` while adding format and DRM details.
  - **Q2 Answer (Code Duplication):** `Book` and `EBook` share metadata like `authors` and `ISBN`. To avoid duplication without multiple inheritance issues, a common abstract intermediate class (e.g., `PrintableBook` or `BookBase`) or a component class (`BookMetadata`) can be used.
  - `AudioBook` inherits from `ElectronicResource` (digital content with platform/seat access), while `Thesis` inherits directly from `Resource` (typically unpriced/free academic material).

- **Part B: Pricing & Taxes (Q4–Q6)**
  - Hardcover books apply a 20% markup.
  - Bulk discounts are computed in `costFor()`: 10% off for 10+ print items, half-price beyond 50 seats for electronic resources.
  - Tax rates are configurable per resource category in `Budget`, and post-tax costs are used for quota evaluation.

- **Part C: Quotas & Budget (Q7–Q10)**
  - Title limits track distinct resource IDs per category.
  - Order cancellation issues a refund to quota usage and holdings, retaining audit history.
  - Department budgets manage isolated departmental allocations via `addDepartmentBudget`.
  - Year-end rollover calculates unspent amounts and carries forward configurable percentages.

- **Part D: Acquisition (Q11–Q12)**
  - `processBatch` supports all-or-nothing transactional guarantees.
  - Vendor offers resolve to the lowest unit price dynamically upon purchase.

- **Part E: Catalogue (Q13–Q14)**
  - Extended searches filter by author, ISBN/ISSN string presence, and publication year range.
  - Lending tracks active physical borrows (capped by holdings) and open digital sessions (capped by licensed seats).

- **Part F: Money (Q15)**
  - `Money` stores minor units (paise/cents) along with an ISO currency code string (default `"USD"` or `"INR"`).
  - Arithmetic and relational operations across mismatched currency codes throw `std::invalid_argument`.

- **Part G: Code Cleanliness (Q16)**
  - Test suites rely strictly on C++ standard `<cassert>` assertions.

---

## 3. Verification & Build
```bash
cmake -S . -B build
cmake --build build
./build/bookmgmt_tests
./build/demo