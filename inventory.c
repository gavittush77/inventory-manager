/* ─────────────────────────────────────────────────────────────────────────────
 * src/inventory.c
 * C data layer – all I/O uses fread / fwrite / fseek on a flat binary file.
 * Every record is exactly sizeof(Item) bytes, so record n starts at
 * offset  n * sizeof(Item).
 * ───────────────────────────────────────────────────────────────────────────*/

#include <stdio.h>
#include <string.h>
#include "inventory.h"

/* ── Internal helpers ────────────────────────────────────────────────────── */

/**
 * find_record_offset
 * Scans the file for a record whose id matches the requested id.
 *
 * Returns the byte-offset of that record on success, or -1 if not found.
 * When found, the record is written into *out_item (may be NULL if the
 * caller only needs the offset).
 *
 * NOTE: This function intentionally searches ALL records, including
 *       soft-deleted ones, so callers can decide what to do with them.
 */
static long find_record_offset(FILE *fp, int id, Item *out_item)
{
    Item  tmp;
    long  offset = 0;

    rewind(fp);

    while (fread(&tmp, sizeof(Item), 1, fp) == 1) {
        if (tmp.id == id) {
            if (out_item) {
                *out_item = tmp;
            }
            return offset;
        }
        offset += (long)sizeof(Item);
    }

    return -1L;
}

/* ── Public API ──────────────────────────────────────────────────────────── */

/*
 * add_item
 * Opens the file in read-write mode (creates it when it does not exist).
 * Rejects duplicated or non-positive IDs, then appends the new record.
 */
int add_item(const Item item)
{
    FILE *fp;
    Item  existing;

    if (item.id <= 0) {
        return INV_FAIL;
    }

    /* Open for reading + writing; create if absent */
    fp = fopen(INVENTORY_FILE, "r+b");
    if (!fp) {
        /* File does not exist yet – create it */
        fp = fopen(INVENTORY_FILE, "w+b");
        if (!fp) {
            return INV_FAIL;
        }
    }

    /* Reject duplicate IDs (active or deleted – IDs are globally unique) */
    if (find_record_offset(fp, item.id, &existing) >= 0) {
        fclose(fp);
        return INV_FAIL;
    }

    /* Append at the end */
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return INV_FAIL;
    }

    if (fwrite(&item, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return INV_FAIL;
    }

    fclose(fp);
    return INV_OK;
}

/*
 * get_item
 * Retrieves one *active* (non-deleted) record by id.
 */
int get_item(int id, Item *out)
{
    FILE *fp;
    Item  tmp;
    long  off;

    if (id <= 0 || !out) {
        return INV_FAIL;
    }

    fp = fopen(INVENTORY_FILE, "rb");
    if (!fp) {
        return INV_FAIL;
    }

    off = find_record_offset(fp, id, &tmp);
    fclose(fp);

    if (off < 0) {
        return INV_FAIL;          /* not found */
    }
    if (tmp.is_deleted) {
        return INV_FAIL;          /* found but soft-deleted */
    }

    *out = tmp;
    return INV_OK;
}

/*
 * update_item
 * Seeks to the existing record's byte offset and overwrites it in-place.
 * Preserves the original id; forces is_deleted = 0.
 */
int update_item(int id, const Item *updated)
{
    FILE *fp;
    Item  existing;
    Item  to_write;
    long  off;

    if (id <= 0 || !updated) {
        return INV_FAIL;
    }

    fp = fopen(INVENTORY_FILE, "r+b");
    if (!fp) {
        return INV_FAIL;
    }

    off = find_record_offset(fp, id, &existing);
    if (off < 0 || existing.is_deleted) {
        fclose(fp);
        return INV_FAIL;
    }

    /* Build the record we want to write */
    to_write            = *updated;
    to_write.id         = id;          /* id is immutable */
    to_write.is_deleted = 0;

    if (fseek(fp, off, SEEK_SET) != 0) {
        fclose(fp);
        return INV_FAIL;
    }

    if (fwrite(&to_write, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return INV_FAIL;
    }

    fclose(fp);
    return INV_OK;
}

/*
 * delete_item
 * Soft-deletes: seeks to the record and flips is_deleted to 1.
 */
int delete_item(int id)
{
    FILE *fp;
    Item  existing;
    long  off;

    if (id <= 0) {
        return INV_FAIL;
    }

    fp = fopen(INVENTORY_FILE, "r+b");
    if (!fp) {
        return INV_FAIL;
    }

    off = find_record_offset(fp, id, &existing);
    if (off < 0 || existing.is_deleted) {
        fclose(fp);
        return INV_FAIL;
    }

    existing.is_deleted = 1;

    if (fseek(fp, off, SEEK_SET) != 0) {
        fclose(fp);
        return INV_FAIL;
    }

    if (fwrite(&existing, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return INV_FAIL;
    }

    fclose(fp);
    return INV_OK;
}

/*
 * list_items
 * Reads all records sequentially, copies only active ones into buffer[].
 * Returns the count of items placed in the buffer.
 */
int list_items(Item *buffer, int max_items)
{
    FILE *fp;
    Item  tmp;
    int   count = 0;

    if (!buffer || max_items <= 0) {
        return 0;
    }

    fp = fopen(INVENTORY_FILE, "rb");
    if (!fp) {
        return 0;   /* no file yet – zero items is valid */
    }

    while (count < max_items && fread(&tmp, sizeof(Item), 1, fp) == 1) {
        if (!tmp.is_deleted) {
            buffer[count++] = tmp;
        }
    }

    fclose(fp);
    return count;
}
