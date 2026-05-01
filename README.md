# Hybrid Inventory Manager

A console-based inventory management application with a **C data layer** (binary
file persistence via `fread`/`fwrite`/`fseek`) and a **C++ frontend** (OOP
wrapper + STL).

---

## Project Structure

```
inventory_manager/
├── include/
│   ├── inventory.h          # Item struct + C API declarations (extern "C")
│   └── InventoryManager.h   # C++ class declaration
├── src/
│   ├── inventory.c          # C backend  – binary file CRUD
│   ├── InventoryManager.cpp # C++ middle layer – STL vector, std::sort
│   └── main.cpp             # Console UI, input validation, menu loop
├── Makefile
└── README.md
```

---

## Prerequisites

| Tool    | Minimum version |
|---------|-----------------|
| `gcc`   | 4.9+ (C11)      |
| `g++`   | 7+   (C++17)    |
| `make`  | 3.81+           |

On Ubuntu/Debian:

```bash
sudo apt update && sudo apt install build-essential
```

---

## Build

```bash
# From the project root directory:
make
```

The executable is placed at **`bin/inventory_manager`**.

Other targets:

```bash
make run     # build + launch in one step
make clean   # remove build/, bin/, and inventory.dat
make help    # list all targets
```

---

## Run

```bash
./bin/inventory_manager
```

You will be presented with a numbered menu:

```
  ┌─────────────────────────────┐
  │        MAIN MENU            │
  ├─────────────────────────────┤
  │  1. Add item                │
  │  2. View item               │
  │  3. Update item             │
  │  4. Delete item             │
  │  5. List all items          │
  │  6. Exit                    │
  └─────────────────────────────┘
```

Inventory data is persisted in **`inventory.dat`** (flat binary file) in the
current working directory. The file is created automatically on the first `add`.

---

## Design Notes

### Data Layer (C – `inventory.c`)

- Each `Item` record is `sizeof(Item)` bytes; record *n* lives at byte offset
  `n × sizeof(Item)`.
- `add_item` appends a new record after scanning for duplicates.
- `update_item` / `delete_item` use `fseek` to overwrite only the target record
  in-place — O(1) write after O(n) scan.
- `delete_item` is a **soft-delete**: it flips `is_deleted = 1`; the slot is
  never reclaimed (simple, safe, and avoids compaction complexity).
- Duplicate IDs are rejected even for previously-deleted records — IDs are
  globally unique for the lifetime of the data file.

### C++ Layer (`InventoryManager.cpp`)

- `fetchAll()` calls `list_items()`, copies results into a
  **`std::vector<Item>`** (STL element 1), then sorts with **`std::sort`** and a
  lambda comparator (STL element 2) — either by `id` or lexicographically by
  `name`.
- Display helpers format the table with `<iomanip>` (`std::setw`, `std::fixed`,
  `std::setprecision`).

### Input Validation (`main.cpp`)

| Field      | Rule                                    |
|------------|-----------------------------------------|
| ID         | Must be a positive integer (> 0)        |
| Name       | Non-empty after trimming whitespace      |
| Quantity   | Integer ≥ 0                             |
| Price      | Float ≥ 0.0                             |

Any invalid input re-prompts the user until a valid value is entered.

---

## Test Cases

The following test cases were verified manually and with automated piped input:

- **TC-1 · Persistence** – Added three items (Apple Watch SE / id=10,
  Sony WH-1000XM5 / id=20, USB-C Cable 2m / id=30), then exited and
  relaunched. "List all" showed all three items, confirming the binary file
  survives process termination.

- **TC-2 · Update persists** – Updated item #20: kept its name via blank input,
  changed quantity from 8 → 15 and price from $349.00 → $279.00. After
  restarting and viewing item #20, all three changes were correctly reflected.

- **TC-3 · Soft-delete** – Deleted item #30 (USB-C Cable 2m). "List all"
  showed only 2 items; "View item #30" returned *"not found (or has been
  deleted)"*. The deleted record remains in the file but is invisible to all
  read operations.

- **TC-4 · Duplicate ID rejected** – Attempted to add a new item with id=10
  (already occupied by Apple Watch SE). The application printed *"Failed to add
  item. ID #10 may already exist."* and did not create a second record.

- **TC-5 · Input validation gauntlet** – On the Add screen, entered id=`-5`
  (→ "ID must be a positive integer"), blank name (→ "Name cannot be empty"),
  quantity=`-1` (→ "Quantity must be 0 or a positive integer"), price=`-9.99`
  (→ "Price must be 0 or a positive number"). After correcting each field the
  item was added successfully.

- **TC-6 · Sort by name** – Used "List all → sort by Name". The output order
  was Apple Watch SE → Laptop Stand → Sony WH-1000XM5, confirming
  lexicographic ascending sort via `std::sort`.

---

## Limitations & Possible Enhancements

- The data file is never compacted; deleted slots accumulate. A `vacuum`
  operation could rewrite only active records to reclaim space.
- No concurrent access protection (no file locking). Suitable for single-user
  desktop use.
- IDs are caller-assigned; an auto-increment sequence could be added by storing
  a small header at the start of the binary file.
