/* ─────────────────────────────────────────────────────────────────────────────
 * src/main.cpp
 * Console front-end – menu loop + validated input helpers.
 * All user interaction lives here; business logic is in InventoryManager.
 * ───────────────────────────────────────────────────────────────────────────*/

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include "InventoryManager.h"

/* ══════════════════════════════════════════════════════════════════════════════
 * Terminal / cosmetic helpers
 * ════════════════════════════════════════════════════════════════════════════*/

static void clearInputBuffer()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static void printBanner()
{
    std::cout << "\n"
              << "  ╔══════════════════════════════════════╗\n"
              << "  ║      HYBRID INVENTORY MANAGER        ║\n"
              << "  ║   C Backend  ·  C++ Frontend         ║\n"
              << "  ╚══════════════════════════════════════╝\n\n";
}

static void printMenu()
{
    std::cout << "\n"
              << "  ┌─────────────────────────────┐\n"
              << "  │        MAIN MENU            │\n"
              << "  ├─────────────────────────────┤\n"
              << "  │  1. Add item                │\n"
              << "  │  2. View item               │\n"
              << "  │  3. Update item             │\n"
              << "  │  4. Delete item             │\n"
              << "  │  5. List all items          │\n"
              << "  │  6. Exit                    │\n"
              << "  └─────────────────────────────┘\n"
              << "  Choice: ";
}

static void pressEnterToContinue()
{
    std::cout << "\n  [Press ENTER to continue]";
    clearInputBuffer();
}

/* ══════════════════════════════════════════════════════════════════════════════
 * Validated input helpers
 * ════════════════════════════════════════════════════════════════════════════*/

/**
 * promptInt
 * Repeatedly asks for an integer until the user provides one that satisfies
 * the supplied predicate.  errorMsg is shown when the value is out of range.
 */
template<typename Pred>
static int promptInt(const std::string &prompt,
                     const std::string &errorMsg,
                     Pred               predicate)
{
    int value;
    while (true) {
        std::cout << "  " << prompt;
        if (std::cin >> value && predicate(value)) {
            clearInputBuffer();
            return value;
        }
        clearInputBuffer();
        std::cout << "  [!] " << errorMsg << "\n";
    }
}

/**
 * promptFloat
 * Same as promptInt but for floats.
 */
template<typename Pred>
static float promptFloat(const std::string &prompt,
                         const std::string &errorMsg,
                         Pred               predicate)
{
    float value;
    while (true) {
        std::cout << "  " << prompt;
        if (std::cin >> value && predicate(value)) {
            clearInputBuffer();
            return value;
        }
        clearInputBuffer();
        std::cout << "  [!] " << errorMsg << "\n";
    }
}

/**
 * promptName
 * Reads a non-empty name, strips leading/trailing whitespace,
 * and ensures it fits within NAME_MAX_LEN - 1 characters.
 */
static std::string promptName()
{
    std::string line;
    while (true) {
        std::cout << "  Name (max " << (NAME_MAX_LEN - 1) << " chars): ";
        std::getline(std::cin, line);

        /* Trim leading whitespace */
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            std::cout << "  [!] Name cannot be empty.\n";
            continue;
        }
        line = line.substr(start);

        /* Trim trailing whitespace */
        size_t end = line.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) {
            line = line.substr(0, end + 1);
        }

        if (line.empty()) {
            std::cout << "  [!] Name cannot be empty.\n";
            continue;
        }
        if (line.size() >= NAME_MAX_LEN) {
            std::cout << "  [!] Name too long (max " << (NAME_MAX_LEN - 1) << " chars).\n";
            continue;
        }
        return line;
    }
}

/**
 * promptSortField
 * Asks the user how to sort the listing.
 */
static SortField promptSortField()
{
    std::cout << "\n  Sort by:  1) ID   2) Name  [default=1]: ";
    int choice;
    if (std::cin >> choice && choice == 2) {
        clearInputBuffer();
        return SortField::BY_NAME;
    }
    clearInputBuffer();
    return SortField::BY_ID;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * Menu action handlers
 * ════════════════════════════════════════════════════════════════════════════*/

static void handleAdd(InventoryManager &mgr)
{
    std::cout << "\n  ── Add New Item ──────────────────────\n";

    Item it{};

    it.id = promptInt(
        "Item ID (positive integer): ",
        "ID must be a positive integer.",
        [](int v){ return v > 0; }
    );

    it.name[0] = '\0';
    std::string name = promptName();
    std::strncpy(it.name, name.c_str(), NAME_MAX_LEN - 1);
    it.name[NAME_MAX_LEN - 1] = '\0';

    it.quantity = promptInt(
        "Quantity (0 or more): ",
        "Quantity must be 0 or a positive integer.",
        [](int v){ return v >= 0; }
    );

    it.price = promptFloat(
        "Price (0.00 or more): ",
        "Price must be 0 or a positive number.",
        [](float v){ return v >= 0.0f; }
    );

    it.is_deleted = 0;

    if (mgr.addItem(it)) {
        std::cout << "\n  [✓] Item #" << it.id << " added successfully.\n";
    } else {
        std::cout << "\n  [✗] Failed to add item. "
                     "ID #" << it.id << " may already exist.\n";
    }
}

static void handleView(InventoryManager &mgr)
{
    std::cout << "\n  ── View Item ─────────────────────────\n";

    int id = promptInt(
        "Enter item ID to view: ",
        "ID must be a positive integer.",
        [](int v){ return v > 0; }
    );

    Item found{};
    if (mgr.getItem(id, found)) {
        std::cout << '\n';
        mgr.printItem(found);
    } else {
        std::cout << "\n  [✗] Item #" << id
                  << " not found (or has been deleted).\n";
    }
}

static void handleUpdate(InventoryManager &mgr)
{
    std::cout << "\n  ── Update Item ───────────────────────\n";

    int id = promptInt(
        "Enter item ID to update: ",
        "ID must be a positive integer.",
        [](int v){ return v > 0; }
    );

    /* Show current state first */
    Item current{};
    if (!mgr.getItem(id, current)) {
        std::cout << "\n  [✗] Item #" << id
                  << " not found (or has been deleted).\n";
        return;
    }

    std::cout << "  Current record:\n";
    mgr.printItem(current);
    std::cout << "  Enter new values (press ENTER on name to keep \""
              << current.name << "\"):\n\n";

    /* Build updated record – start from a blank slate */
    Item updated{};
    updated.id = id;

    /* Name: allow keeping current by entering blank */
    std::cout << "  New name (max " << (NAME_MAX_LEN - 1)
              << " chars, ENTER to keep): ";
    std::string line;
    std::getline(std::cin, line);

    /* Trim */
    size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        /* Blank – keep current */
        std::strncpy(updated.name, current.name, NAME_MAX_LEN - 1);
    } else {
        line = line.substr(start);
        size_t end = line.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) line = line.substr(0, end + 1);

        if (line.size() >= NAME_MAX_LEN) {
            std::cout << "  [!] Name too long; keeping original.\n";
            std::strncpy(updated.name, current.name, NAME_MAX_LEN - 1);
        } else {
            std::strncpy(updated.name, line.c_str(), NAME_MAX_LEN - 1);
        }
    }
    updated.name[NAME_MAX_LEN - 1] = '\0';

    updated.quantity = promptInt(
        "New quantity (0 or more): ",
        "Quantity must be 0 or a positive integer.",
        [](int v){ return v >= 0; }
    );

    updated.price = promptFloat(
        "New price (0.00 or more): ",
        "Price must be 0 or a positive number.",
        [](float v){ return v >= 0.0f; }
    );

    updated.is_deleted = 0;

    if (mgr.updateItem(id, updated)) {
        std::cout << "\n  [✓] Item #" << id << " updated successfully.\n";
    } else {
        std::cout << "\n  [✗] Failed to update item #" << id << ".\n";
    }
}

static void handleDelete(InventoryManager &mgr)
{
    std::cout << "\n  ── Delete Item ───────────────────────\n";

    int id = promptInt(
        "Enter item ID to delete: ",
        "ID must be a positive integer.",
        [](int v){ return v > 0; }
    );

    /* Confirm */
    Item current{};
    if (!mgr.getItem(id, current)) {
        std::cout << "\n  [✗] Item #" << id
                  << " not found (or already deleted).\n";
        return;
    }

    std::cout << "  You are about to delete:\n";
    mgr.printItem(current);
    std::cout << "  Confirm? (y/N): ";

    std::string confirm;
    std::getline(std::cin, confirm);

    if (confirm != "y" && confirm != "Y") {
        std::cout << "  Deletion cancelled.\n";
        return;
    }

    if (mgr.deleteItem(id)) {
        std::cout << "\n  [✓] Item #" << id << " deleted.\n";
    } else {
        std::cout << "\n  [✗] Failed to delete item #" << id << ".\n";
    }
}

static void handleList(InventoryManager &mgr)
{
    std::cout << "\n  ── All Active Items ──────────────────\n";
    SortField sf = promptSortField();
    std::cout << '\n';
    mgr.printAll(sf);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * Entry point
 * ════════════════════════════════════════════════════════════════════════════*/

int main()
{
    InventoryManager mgr;
    printBanner();

    int choice = 0;

    while (true) {
        printMenu();

        if (!(std::cin >> choice)) {
            clearInputBuffer();
            std::cout << "  [!] Invalid input – enter a number 1-6.\n";
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: handleAdd   (mgr); break;
            case 2: handleView  (mgr); break;
            case 3: handleUpdate(mgr); break;
            case 4: handleDelete(mgr); break;
            case 5: handleList  (mgr); break;
            case 6:
                std::cout << "\n  Goodbye!\n\n";
                return 0;
            default:
                std::cout << "  [!] Invalid choice – please enter 1-6.\n";
                break;
        }

        pressEnterToContinue();
    }
}
