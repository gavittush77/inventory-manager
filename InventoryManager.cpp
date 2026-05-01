/* ─────────────────────────────────────────────────────────────────────────────
 * src/InventoryManager.cpp
 * C++ middle layer – wraps the C backend in an OO class and uses STL.
 * ───────────────────────────────────────────────────────────────────────────*/

#include "InventoryManager.h"

#include <algorithm>   /* std::sort                      */
#include <iomanip>     /* std::setw, std::left, etc.     */
#include <iostream>
#include <string>

/* ── Constructor / Destructor ────────────────────────────────────────────── */

InventoryManager::InventoryManager() = default;
InventoryManager::~InventoryManager() = default;

/* ── CRUD wrappers ───────────────────────────────────────────────────────── */

bool InventoryManager::addItem(const Item &item)
{
    return add_item(item) == INV_OK;
}

bool InventoryManager::getItem(int id, Item &out) const
{
    return get_item(id, &out) == INV_OK;
}

bool InventoryManager::updateItem(int id, const Item &updated)
{
    return update_item(id, &updated) == INV_OK;
}

bool InventoryManager::deleteItem(int id)
{
    return delete_item(id) == INV_OK;
}

/* ── Listing with STL ────────────────────────────────────────────────────── */

/**
 * fetchAll
 * Calls the C layer, loads results into a std::vector<Item>, then sorts
 * by the requested field before returning.
 */
std::vector<Item> InventoryManager::fetchAll(SortField field) const
{
    Item raw[MAX_BUFFER];
    int  count = list_items(raw, MAX_BUFFER);

    std::vector<Item> items(raw, raw + count);   /* STL element 1: vector    */

    /* STL element 2: std::sort with a lambda comparator */
    if (field == SortField::BY_NAME) {
        std::sort(items.begin(), items.end(),
                  [](const Item &a, const Item &b) {
                      return std::string(a.name) < std::string(b.name);
                  });
    } else {   /* SortField::BY_ID (default) */
        std::sort(items.begin(), items.end(),
                  [](const Item &a, const Item &b) {
                      return a.id < b.id;
                  });
    }

    return items;
}

/* ── Display helpers ─────────────────────────────────────────────────────── */

static void printDivider(char c = '-', int w = 72)
{
    std::cout << std::string(w, c) << '\n';
}

static void printHeader()
{
    printDivider('=');
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Name"
              << std::setw(12) << "Quantity"
              << std::setw(12) << "Price ($)"
              << '\n';
    printDivider();
}

static void printRow(const Item &it)
{
    std::cout << std::left
              << std::setw(6)  << it.id
              << std::setw(22) << it.name
              << std::setw(12) << it.quantity
              << std::fixed << std::setprecision(2)
              << std::setw(12) << it.price
              << '\n';
}

void InventoryManager::printItem(const Item &item) const
{
    printHeader();
    printRow(item);
    printDivider('=');
}

void InventoryManager::printAll(SortField field) const
{
    std::vector<Item> items = fetchAll(field);

    if (items.empty()) {
        std::cout << "  (no active items in inventory)\n";
        return;
    }

    printHeader();
    for (const auto &it : items) {
        printRow(it);
    }
    printDivider('=');
    std::cout << "  Total: " << items.size() << " item(s)\n";
}
