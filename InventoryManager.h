#ifndef INVENTORY_MANAGER_H
#define INVENTORY_MANAGER_H

/* ─────────────────────────────────────────────────────────────────────────────
 * include/InventoryManager.h
 * C++ class declaration – thin OO wrapper around the C backend API.
 * ───────────────────────────────────────────────────────────────────────────*/

#include <vector>
#include "inventory.h"   /* Item, INV_OK, INV_FAIL, C function prototypes */

/* Sorting options exposed to callers */
enum class SortField {
    BY_ID,
    BY_NAME
};

class InventoryManager {
public:
    InventoryManager();
    ~InventoryManager();

    /* ── CRUD ── */
    bool addItem   (const Item &item);
    bool getItem   (int id, Item &out) const;
    bool updateItem(int id, const Item &updated);
    bool deleteItem(int id);

    /* ── Listing ── */
    /** Returns all active items sorted by the requested field. */
    std::vector<Item> fetchAll(SortField field = SortField::BY_ID) const;

    /* ── Display helpers ── */
    void printItem(const Item &item) const;
    void printAll (SortField field = SortField::BY_ID) const;
};

#endif /* INVENTORY_MANAGER_H */
