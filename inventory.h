#ifndef INVENTORY_H
#define INVENTORY_H

/* ─────────────────────────────────────────────────────────────────────────────
 * inventory.h
 * Shared header for the C data layer and the C++ frontend.
 * All C linkage symbols are wrapped in extern "C" so C++ TUs can include this
 * file without name-mangling issues.
 * ───────────────────────────────────────────────────────────────────────────*/

#define INVENTORY_FILE  "inventory.dat"
#define NAME_MAX_LEN    40
#define MAX_BUFFER      256

/* ── Data record ─────────────────────────────────────────────────────────── */
typedef struct {
    int   id;
    char  name[NAME_MAX_LEN];
    int   quantity;
    float price;
    int   is_deleted;   /* 0 = active, 1 = soft-deleted */
} Item;

/* ── Return-code conventions ─────────────────────────────────────────────── */
#define INV_OK          1
#define INV_FAIL        0

/* ── C backend API ───────────────────────────────────────────────────────── */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * add_item      – Appends a new item to the binary store.
 *                 Returns INV_FAIL if the ID already exists or is <= 0.
 */
int add_item(const Item item);

/**
 * get_item      – Loads the active item with the given id into *out.
 *                 Returns INV_FAIL if not found or soft-deleted.
 */
int get_item(int id, Item *out);

/**
 * update_item   – Overwrites the record that matches id with updated fields.
 *                 The stored id is preserved; is_deleted is set to 0.
 *                 Returns INV_FAIL if not found or already deleted.
 */
int update_item(int id, const Item *updated);

/**
 * delete_item   – Marks the matching record as deleted (soft-delete).
 *                 Returns INV_FAIL if not found or already deleted.
 */
int delete_item(int id);

/**
 * list_items    – Copies up to max_items *active* records into buffer[].
 *                 Returns the number of items actually copied.
 */
int list_items(Item *buffer, int max_items);

#ifdef __cplusplus
}
#endif

#endif /* INVENTORY_H */
